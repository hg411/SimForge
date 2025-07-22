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
#include "Input.h"

SPH2DFluid::SPH2DFluid() {}

SPH2DFluid::~SPH2DFluid() {}

void SPH2DFluid::Init() {
    _threadGroupCountX = static_cast<uint32>(ceil(_maxParticles / _numThreadsX));

    Simulation::InitImgui();
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

    if (INPUT->GetButtonDown(KEY_TYPE::SPACE)) {
        isRunning = !isRunning;
    }
}

void SPH2DFluid::FinalUpdate() {
    Simulation::FinalUpdate();

    //_accumulatedTime += DELTA_TIME;

    //// 초기 프레임 안정화
    //if (_accumulatedTime > 1.0f)
    //    _accumulatedTime = 0.0f;

    //while (_accumulatedTime >= _timeStep) {
    //    
    //}

    if (!isRunning)
        return;

    UpdateSimulationParams();

    ActivateParticles();
    HashingParticles();
    SortParticles();
    ComputeCellRange();
    ComputeDensity();
    PredictPositionVelocity();
    IterativeEOS(5);
    FinalEOS();
    AnimateParticles();

    _accumulatedTime -= _timeStep;

    GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();
}

void SPH2DFluid::Render() {
    Simulation::Render();

    _positionBuffer->SetGraphicsRootSRV(SRV_REGISTER::t0, false);
    _velocityBuffer->SetGraphicsRootSRV(SRV_REGISTER::t1, false);
    _aliveBuffer->SetGraphicsRootSRV(SRV_REGISTER::t2, false);

    _simulationParamsCB->SetGraphicsRootCBV(CBV_REGISTER::b3);

    _particleRenderShader->Update();

    GRAPHICS_CMD_LIST->DrawInstanced(_maxParticles, 1, 0, 0);

    _imgui->Render();
}

void SPH2DFluid::InitShaders() {
    // Line Mesh
    GET_SINGLE(Resources)->CreateLineShader();

    auto CreateCompute = [](shared_ptr<Shader> &shader, const wstring &name) {
        shader = make_shared<Shader>();
        shader->CreateComputeShader(L"../Resources/Shaders/SPH/" + name);
    };

    CreateCompute(_activateShader, L"SPH2DFluidActivateCS.hlsl");
    CreateCompute(_hashShader, L"SPHHashCS.hlsl");
    CreateCompute(_bitonicSortShader, L"SPHBitonicSortCS.hlsl");
    CreateCompute(_cellRangeShader, L"SPHCellRangeCS.hlsl");
    CreateCompute(_densityShader, L"SPHDensityCS.hlsl");
    CreateCompute(_predictShader, L"SPHPredictCS.hlsl");
    CreateCompute(_iterativeEOS1Shader, L"SPHIterativeEOS1CS.hlsl");
    CreateCompute(_iterativeEOS2Shader, L"SPHIterativeEOS2CS.hlsl");
    CreateCompute(_iterativeEOS3Shader, L"SPHIterativeEOS3CS.hlsl");
    CreateCompute(_finalEOSShader, L"SPHFinalCS.hlsl");
    CreateCompute(_animate1Shader, L"SPHAnimation1CS.hlsl");
    CreateCompute(_animate2Shader, L"SPHAnimation2CS.hlsl");
    CreateCompute(_animate3Shader, L"SPHAnimation3CS.hlsl");

    ShaderInfo info = {
        SHADER_TYPE::FORWARD, RASTERIZER_TYPE::CULL_NONE,       DEPTH_STENCIL_TYPE::LESS,
        BLEND_TYPE::DEFAULT,  D3D_PRIMITIVE_TOPOLOGY_POINTLIST, {} // No InputLayout
    };
    _particleRenderShader = make_shared<Shader>();
    _particleRenderShader->CreateVertexShader(L"../Resources/Shaders/SPH/ParticleVS.hlsl");
    _particleRenderShader->CreateGeometryShader(L"../Resources/Shaders/SPH/BillboardGS.hlsl");
    _particleRenderShader->CreatePixelShader(L"../Resources/Shaders/SPH/ParticlePS.hlsl");
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

    _bitonicSortCBs.resize(constsCPU.size());
    for (size_t i = 0; i < constsCPU.size(); i++) {
        _bitonicSortCBs[i] = make_shared<ConstantBuffer>();
        _bitonicSortCBs[i]->Init(sizeof(BitonicSortConsts), 1);
        _bitonicSortCBs[i]->UpdateData(&constsCPU[i], sizeof(BitonicSortConsts));
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
        obj->AddComponent(transform);

        shared_ptr<Camera> camera = make_shared<Camera>();
        camera->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
        obj->AddComponent(camera);

        //shared_ptr<CameraController> cameraController = make_shared<CameraController>();
        //obj->AddComponent(cameraController);

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

    ImGui::Begin("Particles");
    ImGui::InputFloat("Delta Time", &_deltaTime, 0.001f, 0.001f);
    ImGui::InputFloat("Pressure Coeff", &_pressureCoeff, 0.125f, 0.125f);
    ImGui::InputFloat("Near Pressure Coeff", &_nearPressureCoeff, 0.01f, 0.01f);
    ImGui::InputFloat("Viscosity", &_viscosity, 0.0001f, 0.0001f, "%.4f");
    ImGui::InputFloat("Target Density", &_density0, 200.0f, 200.0f);
    ImGui::InputFloat("Mass", &_mass, 0.01f, 0.01f);
    ImGui::End();
}

void SPH2DFluid::UpdateSimulationParams() {
    _sph2DFluidParams.maxParticles = _maxParticles;
    _sph2DFluidParams.deltaTime = _deltaTime;
    _sph2DFluidParams.totalTime += _deltaTime;
    _sph2DFluidParams.pressureCoeff = _pressureCoeff;
    _sph2DFluidParams.nearPressureCoeff = _nearPressureCoeff;
    _sph2DFluidParams.viscosity = _viscosity;
    _sph2DFluidParams.density0 = _density0;
    _sph2DFluidParams.mass = _mass;
    _sph2DFluidParams.smoothingLength = _smoothingLength;
    _sph2DFluidParams.boxCenter = _boxCenter;
    _sph2DFluidParams.boxWidth = _boxWidth;
    _sph2DFluidParams.boxHeight = _boxHeight;
    _sph2DFluidParams.boxDepth = _boxDepth;
    _sph2DFluidParams.cellSize = _cellSize;
    float epsilon = 1e-2f;
    _sph2DFluidParams.gridOrigin = _boxCenter - Vec3(_boxWidth, _boxHeight, _boxDepth) * 0.5f - Vec3(epsilon);
    _sph2DFluidParams.hashCount = _hashCount;
    _sph2DFluidParams.addCount = 1;
    _sph2DFluidParams.radius = _radius;

    _simulationParamsCB->UpdateData(&_sph2DFluidParams, sizeof(_sph2DFluidParams));
}

void SPH2DFluid::ActivateParticles() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _velocityBuffer->BindUAVToCompute(UAV_REGISTER::u1);
    _aliveBuffer->BindUAVToCompute(UAV_REGISTER::u2);

    GEngine->GetComputeDescHeap()->CommitTable();

    // Shader Set
    _activateShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::HashingParticles() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t1);

    _hashBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _cellRangeBuffer->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    // Shader Set
    _hashShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_hashBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::SortParticles() {
    size_t constCount = 0;

    _bitonicSortShader->Update();

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());

    for (uint32_t k = 2; k <= _maxParticles; k *= 2) {
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);
            _bitonicSortCBs[constCount++]->BindToCompute(CBV_REGISTER::b1);

            _positionBuffer->BindUAVToCompute(UAV_REGISTER::u0);
            _velocityBuffer->BindUAVToCompute(UAV_REGISTER::u1);
            _aliveBuffer->BindUAVToCompute(UAV_REGISTER::u2);
            _hashBuffer->BindUAVToCompute(UAV_REGISTER::u3);

            GEngine->GetComputeDescHeap()->CommitTable();

            COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

            COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
        }
    }
}

void SPH2DFluid::ComputeCellRange() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _hashBuffer->BindSRVToCompute(SRV_REGISTER::t0);

    _cellRangeBuffer->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    _cellRangeShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_cellRangeBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::ComputeDensity() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t1);
    _cellRangeBuffer->BindSRVToCompute(SRV_REGISTER::t2);

    _densityBuffer->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    _densityShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::PredictPositionVelocity() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _velocityBuffer->BindSRVToCompute(SRV_REGISTER::t1);
    _densityBuffer->BindSRVToCompute(SRV_REGISTER::t2);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t3);
    _cellRangeBuffer->BindSRVToCompute(SRV_REGISTER::t4);

    _predPositionBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _predVelocityBuffer->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    _predictShader->Update();

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
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _predPositionBuffer->BindSRVToCompute(SRV_REGISTER::t1);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t2);
    _cellRangeBuffer->BindSRVToCompute(SRV_REGISTER::t3);

    _densityBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _pressureBuffer->BindUAVToCompute(UAV_REGISTER::u1);
    _nearPressureBuffer->BindUAVToCompute(UAV_REGISTER::u2);

    GEngine->GetComputeDescHeap()->CommitTable();

    _iterativeEOS1Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::IterativeEOS2() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _positionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _predPositionBuffer->BindSRVToCompute(SRV_REGISTER::t1);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t2);
    _cellRangeBuffer->BindSRVToCompute(SRV_REGISTER::t3);
    _densityBuffer->BindSRVToCompute(SRV_REGISTER::t4);
    _pressureBuffer->BindSRVToCompute(SRV_REGISTER::t5);
    _nearPressureBuffer->BindSRVToCompute(SRV_REGISTER::t6);

    _forceBuffer->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    _iterativeEOS2Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_forceBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::IterativeEOS3() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _forceBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t1);

    _predPositionBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _predVelocityBuffer->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    _iterativeEOS3Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_predPositionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::FinalEOS() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _predPositionBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _predVelocityBuffer->BindSRVToCompute(SRV_REGISTER::t1);
    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t2);

    _positionBuffer->BindUAVToCompute(UAV_REGISTER::u0);
    _velocityBuffer->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    _finalEOSShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);
    
    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH2DFluid::AnimateParticles() {
    _simulationParamsCB->BindToCompute(CBV_REGISTER::b0);

    _aliveBuffer->BindSRVToCompute(SRV_REGISTER::t0);
    _velocityBuffer->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    if (INPUT->GetButton(KEY_TYPE::UP)) {
        _animate1Shader->Update();

        COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

        D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_velocityBuffer->GetBuffer().Get());
        COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
    }

    if (INPUT->GetButton(KEY_TYPE::RIGHT)) {
        _animate2Shader->Update();

        COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

        D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_velocityBuffer->GetBuffer().Get());
        COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
    }

    if (INPUT->GetButton(KEY_TYPE::LEFT)) {
        _animate3Shader->Update();

        COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

        D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_velocityBuffer->GetBuffer().Get());
        COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
    }
}
