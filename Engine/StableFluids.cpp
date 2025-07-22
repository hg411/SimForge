#include "pch.h"
#include "StableFluids.h"
#include "Engine.h"
#include "SimulationObject.h"
#include "Transform.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "Timer.h"
#include "Input.h"
#include "TableDescriptorHeap.h"
#include "CommandQueue.h"

StableFluids::StableFluids() {}

StableFluids::~StableFluids() {}

void StableFluids::Init() {
    const WindowInfo &windowInfo = GEngine->GetWindowInfo();
    _width = windowInfo.width;
    _height = windowInfo.height;

    Simulation::InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitTextures();
    InitSimulationObjects();
}

void StableFluids::Update() { Simulation::Update(); }

void StableFluids::FinalUpdate() {
    Simulation::FinalUpdate();

    UpdateSimulationParams();

    Sourcing();
}

void StableFluids::Render() {}

void StableFluids::InitShaders() {
    auto CreateCompute = [](shared_ptr<Shader> &shader, const wstring &name) {
        shader = make_shared<Shader>();
        shader->CreateComputeShader(L"../Resources/Shaders/StableFluids/" + name);
    };

    CreateCompute(_advectionCS, L"AdvectionCS.hlsl");
    CreateCompute(_applyPressureCS, L"ApplyPressureCS.hlsl");
    CreateCompute(_diffuseCS, L"DiffuseCS.hlsl");
    CreateCompute(_divergenceCS, L"DivergenceCS.hlsl");
    CreateCompute(_jacobiCS, L"JacobiCS.hlsl");
    CreateCompute(_sourcingCS, L"SourcingCS.hlsl");
    CreateCompute(_computeVorticityCS, L"ComputeVorticityCS.hlsl");
    CreateCompute(_confineVorticityCS, L"ConfineVorticityCS.hlsl");
}

void StableFluids::InitConstantBuffers() {
    _stableFluidsParamsCB = make_shared<ConstantBuffer>();
    _stableFluidsParamsCB->Init(sizeof(StableFluidsParams), 1);
}

void StableFluids::InitTextures() {
    auto CreateRWTexture2D = [&](DXGI_FORMAT format) -> shared_ptr<Texture> {
        auto texture = make_shared<Texture>();
        texture->Create(format, _width, _height, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        return texture;
    };

    // R16G16
    _velocity = CreateRWTexture2D(DXGI_FORMAT_R16G16_FLOAT);
    _velocityTemp = CreateRWTexture2D(DXGI_FORMAT_R16G16_FLOAT);

    // R16
    _pressure = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _pressureTemp = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _vorticity = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _divergence = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);

    // R16G16B16A16
    _density = CreateRWTexture2D(DXGI_FORMAT_R16G16B16A16_FLOAT);
    _densityTemp = CreateRWTexture2D(DXGI_FORMAT_R16G16B16A16_FLOAT);
}

void StableFluids::InitSimulationObjects() {
    // Main Camera
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"Main_Camera");

        shared_ptr<Transform> transform = make_shared<Transform>();
        obj->AddComponent(transform);

        shared_ptr<Camera> camera = make_shared<Camera>();
        camera->SetProjectionType(PROJECTION_TYPE::ORTHOGRAPHIC);
        obj->AddComponent(camera);

        AddSimulationObject(obj);
    }
}

void StableFluids::BuildUI() {
    // ImGui::Begin("Color");
    // ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    //  ImGui::Checkbox()
    // ImGui::End();
}

void StableFluids::UpdateSimulationParams() {
    static int color = 0;
    static bool prevLeftButton = false;
    static Vec2 prevMouseNdc = Vec2(-1.0f, -1.0f);

    _stableFluidsParams.dt = DELTA_TIME;
    _stableFluidsParams.viscosity = 0.001f;

    if (INPUT->GetButton(KEY_TYPE::LEFT)) {
        _stableFluidsParams.i = static_cast<uint32>(INPUT->GetMousePos().x);
        _stableFluidsParams.j = static_cast<uint32>(INPUT->GetMousePos().y);

        if (!prevLeftButton) { // 버튼을 새로 누른 경우.
                               // 랜덤하게 색 교체
            static const std::vector<Vec4> rainbow = {
                {1.0f, 0.0f, 0.0f, 1.0f},  // Red
                {1.0f, 0.65f, 0.0f, 1.0f}, // Orange
                {1.0f, 1.0f, 0.0f, 1.0f},  // Yellow
                {0.0f, 1.0f, 0.0f, 1.0f},  // Green
                {0.0f, 0.0f, 1.0f, 1.0f},  // Blue
                {0.3f, 0.0f, 0.5f, 1.0f},  // Indigo
                {0.5f, 0.0f, 1.0f, 1.0f}   // Violet/Purple
            };

            _stableFluidsParams.sourcingDensity = rainbow[(color++) & 7];
            _stableFluidsParams.sourcingVelocity = Vec2(0.0f);
        } else {
            Vec2 ndcVel = INPUT->GetMouseNdc() - prevMouseNdc;
            _stableFluidsParams.sourcingVelocity = ndcVel * 10.0f;
        }
    } else {
        _stableFluidsParams.i = -1; // uint의 Overflow 이용(좌클릭을 누르지 않을때 CS에서 처리)
    }

    prevLeftButton = INPUT->GetButton(KEY_TYPE::LEFT);
    prevMouseNdc = INPUT->GetMouseNdc();

    _stableFluidsParamsCB->UpdateData(&_stableFluidsParams, sizeof(_stableFluidsParams));
}

void StableFluids::Sourcing() { 
    _stableFluidsParamsCB->BindToCompute(CBV_REGISTER::b0);

    _velocity->BindUAVToCompute(UAV_REGISTER::u0);
    _density->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    _sourcingCS->Update();

    COMPUTE_CMD_LIST->Dispatch(UINT(ceil(_width / 32.0f)), UINT(ceil(_height / 32.0f)), 1);

    D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get()),
                                            CD3DX12_RESOURCE_BARRIER::UAV(_density->GetTex2D().Get())};

    COMPUTE_CMD_LIST->ResourceBarrier(1, uavBarriers);
}
