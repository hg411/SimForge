#include "pch.h"
#include "SPH3DFluid.h"
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

#include "Device.h"

SPH3DFluid::SPH3DFluid() {}

SPH3DFluid::~SPH3DFluid() {}

void SPH3DFluid::Init() {
    _threadGroupCountX = static_cast<uint32>(ceil(_maxParticles / _numThreadsX));
    _smoothingLength = 1.5f * _radius * 2.0f;
    _cellSize = 1.2f * _smoothingLength;
    _hashCount = _maxParticles;

    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitStructuredBuffers();
    InitSimulationObjects();

    PushSimulationParams();
    ActivateParticles1();
    GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();
}

void SPH3DFluid::Update() {
    Simulation::Update();

    for (auto &obj : _simulationObjects) {
        if (obj->GetName() == L"BoundingBox") {
            auto transform = obj->GetTransform();
            if (transform) {
                transform->SetLocalScale(Vec3(_boxWidth, _boxHeight, _boxDepth));
                transform->SetLocalPosition(_boxCenter);
            }
        }
    }

    if (INPUT->GetButtonDown(KEY_TYPE::SPACE)) {
        isRunning = !isRunning;
    }
}

void SPH3DFluid::FinalUpdate() {
    Simulation::FinalUpdate();

    if (!isRunning)
        return;

    //_accumulatedTime += DELTA_TIME;

    //// 초기 프레임 안정화
    // if (_accumulatedTime > 1.0f)
    //     _accumulatedTime = 0.0f;

    // while (_accumulatedTime >= _timeStep) {
    //     PushSimulationParams();

    //    if (INPUT->GetButtonDown(KEY_TYPE::KEY_1)) {
    //        ActivateParticles1();
    //    }

    //    if (INPUT->GetButtonDown(KEY_TYPE::KEY_2)) {
    //        ActivateParticles2();
    //    }

    //    HashingParticles();
    //    SortParticles();
    //    ComputeCellRange();
    //    ComputeDensity();
    //    PredictPositionVelocity();
    //    IterativeEOS(3);
    //    FinalEOS();
    //    AnimateParticles();

    //    _accumulatedTime -= _timeStep;

    //    GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();
    //}

    PushSimulationParams();

    if (INPUT->GetButtonDown(KEY_TYPE::KEY_1)) {
        ActivateParticles1();
    }

    if (INPUT->GetButtonDown(KEY_TYPE::KEY_2)) {
        ActivateParticles2();
    }

    HashingParticles();
    SortParticles();
    ComputeCellRange();
    ComputeDensity();
    PredictPositionVelocity();
    IterativeEOS(3);
    FinalEOS();
    AnimateParticles();

    GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();
}

void SPH3DFluid::Render() {
    Simulation::Render();

    _positionBuffer->SetGraphicsRootSRV(SRV_REGISTER::t0);
    _velocityBuffer->SetGraphicsRootSRV(SRV_REGISTER::t1);
    _aliveBuffer->SetGraphicsRootSRV(SRV_REGISTER::t2);

    _simulationParamsCB->SetGraphicsRootCBV(CBV_REGISTER::b3);

    _particleRenderShader->Update();

    GRAPHICS_CMD_LIST->DrawInstanced(_maxParticles, 1, 0, 0);

    _imgui->Render();
}

void SPH3DFluid::InitShaders() {
    // Line Mesh
    GET_SINGLE(Resources)->CreateLineShader();

    auto CreateCompute = [](shared_ptr<Shader> &shader, const wstring &name) {
        shader = make_shared<Shader>();
        shader->CreateComputeShader(L"../Resources/Shaders/SPH/" + name);
    };

    CreateCompute(_activate1Shader, L"SPH3DActivateParticles1CS.hlsl");
    CreateCompute(_activate2Shader, L"SPH3DActivateParticles2CS.hlsl");
    CreateCompute(_hashShader, L"SPHHashCS.hlsl");
    CreateCompute(_bitonicSortShader, L"SPHBitonicSortCS.hlsl");
    CreateCompute(_cellRangeShader, L"SPHCellRangeCS.hlsl");
    CreateCompute(_densityShader, L"SPH3DDensityCS.hlsl");
    CreateCompute(_predictShader, L"SPH3DPredictCS.hlsl");
    CreateCompute(_iterativeEOS1Shader, L"SPH3DIterativeEOS1CS.hlsl");
    CreateCompute(_iterativeEOS2Shader, L"SPH3DIterativeEOS2CS.hlsl");
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

void SPH3DFluid::InitConstantBuffers() {
    // SPH 3D Fluid Params
    _simulationParamsCB = make_shared<ConstantBuffer>();
    _simulationParamsCB->Init(sizeof(SPH3DFluidParams), 1);

    // Bitonic Sort
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

void SPH3DFluid::InitStructuredBuffers() {
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

void SPH3DFluid::InitSimulationObjects() {
    // Main Camera
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"Main_Camera");

        shared_ptr<Transform> transform = make_shared<Transform>();
        transform->SetLocalPosition(Vec3(0.0f, 0.0f, -1.0f));
        obj->AddComponent(transform);
        shared_ptr<Camera> camera = make_shared<Camera>();
        obj->AddComponent(camera);
        shared_ptr<CameraController> cameraController = make_shared<CameraController>();
        obj->AddComponent(cameraController);

        AddSimulationObject(obj);
    }

    // Bounding Box
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"BoundingBox");

        shared_ptr<Mesh> mesh = GET_SINGLE(Resources)->LoadCubeLineMesh();
        shared_ptr<GraphicsMaterial> material = make_shared<GraphicsMaterial>();
        material->SetAlbedo(Vec3(253.0f, 253.0f, 150.0f) / 255.0f);
        material->SetShader(GET_SINGLE(Resources)->Get<Shader>(L"LineShader"));

        shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
        meshRenderer->SetMesh(mesh);
        meshRenderer->SetMaterial(material);

        shared_ptr<Transform> transform = make_shared<Transform>();
        transform->SetLocalPosition(_boxCenter);
        transform->SetLocalScale(Vec3(_boxWidth, _boxHeight, _boxDepth));

        obj->AddComponent(meshRenderer);
        obj->AddComponent(transform);

        AddSimulationObject(obj);
    }
}

void SPH3DFluid::BuildUI() {
    ImGui::Begin("Bounding Box");
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    ImGui::SliderFloat("Box Width", &_boxWidth, 0.5f, 3.0f);
    ImGui::SliderFloat("Box Height", &_boxHeight, 0.5f, 1.6f);
    ImGui::SliderFloat("Box Depth", &_boxDepth, 0.5f, 1.5f);
    ImGui::SliderFloat3("Box Position", &_boxCenter.x, -2.0f, 2.0f);
    ImGui::End();

    ImGui::Begin("Particles");
    ImGui::InputFloat("Delta Time", &_deltaTime, 0.001f, 0.001f);
    ImGui::InputFloat("Pressure Coeff", &_pressureCoeff, 0.125f, 0.125f);
    ImGui::InputFloat("Near Pressure Coeff", &_nearPressureCoeff, 0.0001f, 0.0001f, "%.4f");
    ImGui::InputFloat("Viscosity", &_viscosity, 0.0001f, 0.0001f, "%.4f");
    ImGui::InputFloat("Target Density", &_density0, 50.0f, 50.0f);
    ImGui::InputFloat("Mass", &_mass, 0.0001f, 0.0001f, "%.4f");
    ImGui::End();
}

void SPH3DFluid::PushSimulationParams() {
    _sph3DFluidParams.maxParticles = _maxParticles;
    _sph3DFluidParams.deltaTime = _deltaTime;
    _sph3DFluidParams.totalTime += _deltaTime;
    _sph3DFluidParams.pressureCoeff = _pressureCoeff;
    _sph3DFluidParams.nearPressureCoeff = _nearPressureCoeff;
    _sph3DFluidParams.viscosity = _viscosity;
    _sph3DFluidParams.density0 = _density0;
    _sph3DFluidParams.mass = _mass;
    _sph3DFluidParams.smoothingLength = _smoothingLength;
    _sph3DFluidParams.boxCenter = _boxCenter;
    _sph3DFluidParams.boxWidth = _boxWidth;
    _sph3DFluidParams.boxHeight = _boxHeight;
    _sph3DFluidParams.boxDepth = _boxDepth;
    _sph3DFluidParams.cellSize = _cellSize;
    float epsilon = 1e-2f;
    _sph3DFluidParams.gridOrigin = _boxCenter - Vec3(_boxWidth, _boxHeight, _boxDepth) * 0.5f - Vec3(epsilon);
    _sph3DFluidParams.hashCount = _hashCount;
    _sph3DFluidParams.addCount = 1;
    _sph3DFluidParams.radius = _radius;

    _simulationParamsCB->UpdateData(&_sph3DFluidParams, sizeof(_sph3DFluidParams));
    _simulationParamsCB->SetComputeRootCBV(CBV_REGISTER::b0);
}

void SPH3DFluid::ActivateParticles1() {
    _positionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _velocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);
    _aliveBuffer->SetComputeRootUAV(UAV_REGISTER::u2);

    // Shader Set
    _activate1Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::ActivateParticles2() {
    _positionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _velocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);
    _aliveBuffer->SetComputeRootUAV(UAV_REGISTER::u2);

    // Shader Set
    _activate2Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::HashingParticles() {
    _positionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _hashBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _cellRangeBuffer->SetComputeRootUAV(UAV_REGISTER::u1);

    // Shader Set
    _hashShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_hashBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::SortParticles() {
    size_t constCount = 0;

    _bitonicSortShader->Update();

    _positionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _velocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);
    _aliveBuffer->SetComputeRootUAV(UAV_REGISTER::u2);
    _hashBuffer->SetComputeRootUAV(UAV_REGISTER::u3);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());

    for (uint32_t k = 2; k <= _maxParticles; k *= 2) {
        for (uint32_t j = k / 2; j > 0; j /= 2) {
            _bitonicSortCBs[constCount++]->SetComputeRootCBV(CBV_REGISTER::b1);

            COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

            COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
        }
    }
}

void SPH3DFluid::ComputeCellRange() {
    _hashBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _cellRangeBuffer->SetComputeRootUAV(UAV_REGISTER::u0);

    _cellRangeShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_cellRangeBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::ComputeDensity() {
    _positionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _cellRangeBuffer->SetComputeRootSRV(SRV_REGISTER::t2);
    _densityBuffer->SetComputeRootUAV(UAV_REGISTER::u0);

    _densityShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::PredictPositionVelocity() {
    _positionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _velocityBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _densityBuffer->SetComputeRootSRV(SRV_REGISTER::t2);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t3);
    _cellRangeBuffer->SetComputeRootSRV(SRV_REGISTER::t4);

    _predPositionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _predVelocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);

    _predictShader->Update();
    ;
    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_predPositionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::IterativeEOS(uint32 iterationCount) {
    for (uint32 i = 0; i < iterationCount; ++i) {
        IterativeEOS1();
        IterativeEOS2();
        IterativeEOS3();
    }
}

void SPH3DFluid::IterativeEOS1() {
    _positionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _predPositionBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t2);
    _cellRangeBuffer->SetComputeRootSRV(SRV_REGISTER::t3);

    _densityBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _pressureBuffer->SetComputeRootUAV(UAV_REGISTER::u1);
    _nearPressureBuffer->SetComputeRootUAV(UAV_REGISTER::u2);

    _iterativeEOS1Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_densityBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::IterativeEOS2() {
    _positionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _predPositionBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t2);
    _cellRangeBuffer->SetComputeRootSRV(SRV_REGISTER::t3);
    _densityBuffer->SetComputeRootSRV(SRV_REGISTER::t4);
    _pressureBuffer->SetComputeRootSRV(SRV_REGISTER::t5);
    _nearPressureBuffer->SetComputeRootSRV(SRV_REGISTER::t6);

    _forceBuffer->SetComputeRootUAV(UAV_REGISTER::u0);

    _iterativeEOS2Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_forceBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::IterativeEOS3() {
    _forceBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t1);

    _predPositionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _predVelocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);

    _iterativeEOS3Shader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_predPositionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::FinalEOS() {
    _predPositionBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _predVelocityBuffer->SetComputeRootSRV(SRV_REGISTER::t1);
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t2);

    _positionBuffer->SetComputeRootUAV(UAV_REGISTER::u0);
    _velocityBuffer->SetComputeRootUAV(UAV_REGISTER::u1);

    _finalEOSShader->Update();

    COMPUTE_CMD_LIST->Dispatch(_threadGroupCountX, 1, 1);

    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(_positionBuffer->GetBuffer().Get());
    COMPUTE_CMD_LIST->ResourceBarrier(1, &uavBarrier);
}

void SPH3DFluid::AnimateParticles() {
    _aliveBuffer->SetComputeRootSRV(SRV_REGISTER::t0);
    _velocityBuffer->SetComputeRootUAV(UAV_REGISTER::u0);

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
