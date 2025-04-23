#include "pch.h"
#include "SPH2DFluid.h"
#include "Resources.h"
#include "Transform.h"
#include "Camera.h"
#include "CameraController.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "StructuredBuffer.h"
#include "TableDescriptorHeap.h"
#include "CommandQueue.h"
#include "Engine.h"
#include "Timer.h"

SPH2DFluid::SPH2DFluid() {}

SPH2DFluid::~SPH2DFluid() {}

void SPH2DFluid::Init() {
    _threadGroupCountX = static_cast<uint32>(ceil(_maxParticles / _numThreadsX));

    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitStructuredBuffers();
    InitSimulationObjects();
}

void SPH2DFluid::Update() {
    Simulation::Update();

    for (auto &obj : _simulationObjects) {
        if (obj->GetName() == L"BoundingBox") {
            auto transform = obj->GetTransform();
            if (transform) {
                transform->SetLocalScale(Vec3(_boxWidth, _boxHeight, 1.0f));
                transform->SetLocalPosition(_boxCenter);
            }
        }
    }
}

void SPH2DFluid::FinalUpdate() {
    Simulation::FinalUpdate();

    _accumulatedTime += DELTA_TIME;

    // 초기 프레임 안정화
    if (_accumulatedTime > 1.0f)
        _accumulatedTime = 0.0f;

    while (_accumulatedTime >= _timeStep) {
        PushSimulationParams();
        ActivateParticles();
        HashingParticles();
        SortParticles();
        ComputeCellRange();
        ComputeDensity();
        PredictPositionVelocity();
        IterativeEOS(5);
        FinalEOS();

        GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();

        _accumulatedTime -= _timeStep;
    }
}

void SPH2DFluid::Render() {
    Simulation::Render();

    _positionBuffer->PushGraphicsData(SRV_REGISTER::t0);
    _velocityBuffer->PushGraphicsData(SRV_REGISTER::t1);
    _aliveBuffer->PushGraphicsData(SRV_REGISTER::t2);

    //_simulationParamsCB->Clear(); // 삭제
    //_simulationParamsCB->PushGraphicsData(&_sph2DFluidParams, sizeof(_sph2DFluidParams), CBV_REGISTER::b3); //삭제
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToGraphics(CBV_REGISTER::b3);

    GEngine->GetGraphicsDescHeap()->CommitTable();

    _particleRenderShader->Update();

    GRAPHICS_CMD_LIST->DrawInstanced(_maxParticles, 1, 0, 0);

    _imgui->Render();
}

void SPH2DFluid::InitShaders() {
    // Line Mesh
    GET_SINGLE(Resources)->CreateLineShader();

    _activateShader = make_shared<Shader>();
    _activateShader->CreateComputeShader(L"SPH2DFluidActivateCS.hlsl");

    _hashShader = make_shared<Shader>();
    _hashShader->CreateComputeShader(L"SPHHashCS.hlsl");

    _bitonicSortShader = make_shared<Shader>();
    _bitonicSortShader->CreateComputeShader(L"SPHBitonicSortCS.hlsl");

    _cellRangeShader = make_shared<Shader>();
    _cellRangeShader->CreateComputeShader(L"SPHCellRangeCS.hlsl");

    _densityShader = make_shared<Shader>();
    _densityShader->CreateComputeShader(L"SPHDensityCS.hlsl");

    _predictShader = make_shared<Shader>();
    _predictShader->CreateComputeShader(L"SPHPredictCS.hlsl");

    _iterativeEOS1Shader = make_shared<Shader>();
    _iterativeEOS1Shader->CreateComputeShader(L"SPHIterativeEOS1CS.hlsl");

    _iterativeEOS2Shader = make_shared<Shader>();
    _iterativeEOS2Shader->CreateComputeShader(L"SPHIterativeEOS2CS.hlsl");

    _iterativeEOS3Shader = make_shared<Shader>();
    _iterativeEOS3Shader->CreateComputeShader(L"SPHIterativeEOS3CS.hlsl");

    _finalEOSShader = make_shared<Shader>();
    _finalEOSShader->CreateComputeShader(L"SPHFinalCS.hlsl");

    ShaderInfo info = {
        SHADER_TYPE::FORWARD, RASTERIZER_TYPE::CULL_NONE,       DEPTH_STENCIL_TYPE::LESS,
        BLEND_TYPE::DEFAULT,  D3D_PRIMITIVE_TOPOLOGY_POINTLIST, {} // No InputLayout
    };
    _particleRenderShader = make_shared<Shader>();
    _particleRenderShader->CreateVertexShader(L"ParticleVS.hlsl");
    _particleRenderShader->CreateGeometryShader(L"BillboardGS.hlsl");
    _particleRenderShader->CreatePixelShader(L"ParticlePS.hlsl");
    _particleRenderShader->CreateGraphicsShader(info);
}

void SPH2DFluid::InitConstantBuffers() {
    _simulationParamsCB = make_shared<ConstantBuffer>();
    _simulationParamsCB->Init(sizeof(SPH2DFluidParams), 1);

    vector<BitonicSortConsts> constsCPU;
    for (uint32_t k = 2; k <= _maxParticles; k *= 2)
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            BitonicSortConsts c;
            c.j = j;
            c.k = k;
            constsCPU.push_back(c);
        }

    _bitonicSortCB = make_shared<ConstantBuffer>();
    _bitonicSortCB->Init(sizeof(BitonicSortConsts), constsCPU.size());

    for (size_t i = 0; i < constsCPU.size(); i++) {
        _bitonicSortCB->SetData(&constsCPU[i], sizeof(BitonicSortConsts));
    }
}

void SPH2DFluid::InitStructuredBuffers() {
    _positionBuffer = make_shared<StructuredBuffer>();
    _positionBuffer->Init(sizeof(Vec3), _maxParticles);

    _velocityBuffer = make_shared<StructuredBuffer>();
    _velocityBuffer->Init(sizeof(Vec3), _maxParticles);

    _forceBuffer = make_shared<StructuredBuffer>();
    _forceBuffer->Init(sizeof(Vec3), _maxParticles);

    _densityBuffer = make_shared<StructuredBuffer>();
    _densityBuffer->Init(sizeof(float), _maxParticles);

    _pressureBuffer = make_shared<StructuredBuffer>();
    _pressureBuffer->Init(sizeof(float), _maxParticles);

    _nearPressureBuffer = make_shared<StructuredBuffer>();
    _nearPressureBuffer->Init(sizeof(float), _maxParticles);

    vector<int> aliveCPU(_maxParticles, -1);
    _aliveBuffer = make_shared<StructuredBuffer>();
    _aliveBuffer->Init(sizeof(int), _maxParticles, aliveCPU.data());

    _hashBuffer = make_shared<StructuredBuffer>();
    _hashBuffer->Init(sizeof(uint32), _maxParticles);

    _cellRangeBuffer = make_shared<StructuredBuffer>();
    _cellRangeBuffer->Init(sizeof(CellRange), _hashCount);

    _predPositionBuffer = make_shared<StructuredBuffer>();
    _predPositionBuffer->Init(sizeof(Vec3), _maxParticles);

    _predVelocityBuffer = make_shared<StructuredBuffer>();
    _predVelocityBuffer->Init(sizeof(Vec3), _maxParticles);
}

void SPH2DFluid::InitSimulationObjects() {
    // Main Camera
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"Main_Camera");

        shared_ptr<Transform> transform = make_shared<Transform>();
        transform->SetLocalPosition(Vec3(0.0f, 0.0f, 0.0f));
        obj->AddComponent(transform);
        shared_ptr<Camera> camera = make_shared<Camera>();
        camera->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
        obj->AddComponent(camera);
        shared_ptr<CameraController> cameraController = make_shared<CameraController>();
        obj->AddComponent(cameraController);

        AddSimulationObject(obj);
    }

    // Bounding Box
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"BoundingBox");

        shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadRectangleLineMesh();
        shared_ptr<GraphicsMaterial> material = make_shared<GraphicsMaterial>();
        material->SetAlbedo(Vec3(253.0f, 253.0f, 150.0f) / 255.0f);
        material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"LineShader"));

        shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
        meshRenderer->SetMesh(mesh);
        meshRenderer->SetMaterial(material);

        shared_ptr<Transform> transform = make_shared<Transform>();
        transform->SetLocalPosition(_boxCenter);
        transform->SetLocalScale(Vec3(_boxWidth, _boxHeight, 1.0f));

        obj->AddComponent(meshRenderer);
        obj->AddComponent(transform);

        AddSimulationObject(obj);
    }
}

void SPH2DFluid::BuildUI() {
    ImGui::Begin("Bounding Box");
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    ImGui::SliderFloat("Box Width", &_boxWidth, 0.5f, 3.4f);
    ImGui::SliderFloat("Box Height", &_boxHeight, 0.5f, 1.7f);
    ImGui::SliderFloat2("Box Position", &_boxCenter.x, -2.0f, 2.0f);
    ImGui::End();
}

void SPH2DFluid::PushSimulationParams() {
    _sph2DFluidParams.maxParticles = _maxParticles;
    _sph2DFluidParams.deltaTime = _deltaTime;
    _sph2DFluidParams.totalTime += _deltaTime;
    _sph2DFluidParams.pressureCoeff = _pressureCoeff;
    _sph2DFluidParams.nearPressureCoeff = _nearPressureCoeff;
    _sph2DFluidParams.viscosity = _viscosity;
    _sph2DFluidParams.density0 = _density0;
    _sph2DFluidParams.mass = _mass;
    _sph2DFluidParams.smoothingLength = _smoothingLength;
    _sph2DFluidParams.boxWidth = _boxWidth;
    _sph2DFluidParams.boxCenter = _boxCenter;
    _sph2DFluidParams.boxHeight = _boxHeight;
    _sph2DFluidParams.gridOrigin = _boxCenter - Vec3(_boxWidth * 0.5f + 1e-3f, _boxHeight * 0.5f + 1e-3f, 1e-3f);
    _sph2DFluidParams.cellSize = _cellSize;
    _sph2DFluidParams.hashCount = _hashCount;
    _sph2DFluidParams.addCount = 1;
    _sph2DFluidParams.radius = _radius;

    //_simulationParamsCB->PushComputeData(&_sph2DFluidParams, sizeof(_sph2DFluidParams), CBV_REGISTER::b0); // 삭제
    _simulationParamsCB->Clear();
    _simulationParamsCB->SetData(&_sph2DFluidParams, sizeof(_sph2DFluidParams));
}

void SPH2DFluid::ActivateParticles() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeUAVData(UAV_REGISTER::u0);
    _velocityBuffer->PushComputeUAVData(UAV_REGISTER::u1);
    _aliveBuffer->PushComputeUAVData(UAV_REGISTER::u2);

    // Shader Set
    _activateShader->Update();

    // dispatch
    GEngine->GetComputeDescHeap()->CommitTable();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::HashingParticles() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _hashBuffer->PushComputeUAVData(UAV_REGISTER::u0);

    // Shader Set
    _hashShader->Update();

    // dispatch
    GEngine->GetComputeDescHeap()->CommitTable();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_hashBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::SortParticles() {
    // Shader Set + Dispatch
    _bitonicSortCB->Clear();

    _bitonicSortShader->Update();

    for (uint32_t k = 2; k <= _maxParticles; k *= 2) {
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            _bitonicSortCB->BindToCompute(CBV_REGISTER::b0);

            _positionBuffer->PushComputeUAVData(UAV_REGISTER::u0);
            _velocityBuffer->PushComputeUAVData(UAV_REGISTER::u1);
            _aliveBuffer->PushComputeUAVData(UAV_REGISTER::u2);
            _hashBuffer->PushComputeUAVData(UAV_REGISTER::u3);
            
            GEngine->GetComputeDescHeap()->CommitTable();
            COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

            D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
            COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
        }
    }
}

void SPH2DFluid::ComputeCellRange() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _hashBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _cellRangeBuffer->PushComputeUAVData(UAV_REGISTER::u0);

    _cellRangeShader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_cellRangeBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::ComputeDensity() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _cellRangeBuffer->PushComputeSRVData(SRV_REGISTER::t2);
    _densityBuffer->PushComputeUAVData(UAV_REGISTER::u0);

    _densityShader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::PredictPositionVelocity() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _velocityBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _densityBuffer->PushComputeSRVData(SRV_REGISTER::t2);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t3);
    _cellRangeBuffer->PushComputeSRVData(SRV_REGISTER::t4);

    _predPositionBuffer->PushComputeUAVData(UAV_REGISTER::u0);
    _predVelocityBuffer->PushComputeUAVData(UAV_REGISTER::u1);

    _predictShader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_predPositionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::IterativeEOS(uint32 iterationCount) {
    for (uint32 i = 0; i < iterationCount; ++i) {
        IterativeEOS1();
        IterativeEOS2();
        IterativeEOS3();
    }
}

void SPH2DFluid::IterativeEOS1() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _predPositionBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t2);
    _cellRangeBuffer->PushComputeSRVData(SRV_REGISTER::t3);

    _densityBuffer->PushComputeUAVData(UAV_REGISTER::u0);
    _pressureBuffer->PushComputeUAVData(UAV_REGISTER::u1);
    _nearPressureBuffer->PushComputeUAVData(UAV_REGISTER::u2);

    _iterativeEOS1Shader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::IterativeEOS2() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _predPositionBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t2);
    _cellRangeBuffer->PushComputeSRVData(SRV_REGISTER::t3);
    _densityBuffer->PushComputeSRVData(SRV_REGISTER::t4);
    _pressureBuffer->PushComputeSRVData(SRV_REGISTER::t5);
    _nearPressureBuffer->PushComputeSRVData(SRV_REGISTER::t6);

    _forceBuffer->PushComputeUAVData(UAV_REGISTER::u0);

    _iterativeEOS2Shader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_forceBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::IterativeEOS3() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _forceBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t1);

    _predPositionBuffer->PushComputeUAVData(UAV_REGISTER::u0);
    _predVelocityBuffer->PushComputeUAVData(UAV_REGISTER::u1);

    _iterativeEOS3Shader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_predPositionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::FinalEOS() {
    _simulationParamsCB->Clear();
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _predPositionBuffer->PushComputeSRVData(SRV_REGISTER::t0);
    _predVelocityBuffer->PushComputeSRVData(SRV_REGISTER::t1);
    _aliveBuffer->PushComputeSRVData(SRV_REGISTER::t2);

    _positionBuffer->PushComputeUAVData(UAV_REGISTER::u0);
    _velocityBuffer->PushComputeUAVData(UAV_REGISTER::u1);

    _finalEOSShader->Update();

    GEngine->GetComputeDescHeap()->CommitTable();
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}
