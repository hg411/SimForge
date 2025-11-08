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
#include "SwapChain.h"
#include "RenderTargetGroup.h"

StableFluids::StableFluids() {}

StableFluids::~StableFluids() {}

void StableFluids::Init() {
    _width = 1024;
    _height = 1024;

    Simulation::InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitTextures();
    InitSimulationObjects();

    GEngine->ResizeWindow(_width, _height);
}

void StableFluids::Update() { Simulation::Update(); }

void StableFluids::FinalUpdate() {
    Simulation::FinalUpdate();

    UpdateSimulationParams();

    Sourcing();
    ComputeVorticity();
    ConfineVorticity();
    Diffuse();
    Projection();
    Advection();

    GEngine->GetComputeCmdQueue()->FlushComputeCommandQueue();
}

void StableFluids::Render() {
    Simulation::Render();

    // 백버퍼에 출력
    // Table 사용 (Texture의 경우 Structured Buffer와 달리 무조건 Descriptor Table을 이용해야 함.)
    //_density->BindSRVToGraphics(SRV_REGISTER::t0, true);
    _density->SetSRVToGraphics(SRV_REGISTER::t0, true);

    GEngine->GetGraphicsDescHeap()->CommitTable();

    _densityRenderShader->Update();

    GRAPHICS_CMD_LIST->DrawInstanced(3, 1, 0, 0);

    // 다음 프레임에서 Compute_CMD_LIST가 SRV(for pixelShader) 상태일 시 Transition 불가.
    _density->Transition(GRAPHICS_CMD_LIST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    _imgui->Render();
}

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

    ShaderInfo info = {
        SHADER_TYPE::FORWARD, RASTERIZER_TYPE::CULL_NONE,          DEPTH_STENCIL_TYPE::LESS,
        BLEND_TYPE::DEFAULT,  D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, {} // No InputLayout
    };
    _densityRenderShader = make_shared<Shader>();
    _densityRenderShader->CreateVertexShader(L"../Resources/Shaders/StableFluids/DensityRenderVS.hlsl");
    _densityRenderShader->CreatePixelShader(L"../Resources/Shaders/StableFluids/DensityRenderPS.hlsl");
    _densityRenderShader->CreateGraphicsShader(info);
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

    // R16G16 FLOAT
    _velocity = CreateRWTexture2D(DXGI_FORMAT_R16G16_FLOAT);
    _velocityTemp = CreateRWTexture2D(DXGI_FORMAT_R16G16_FLOAT);

    // R16 FLOAT
    _pressure = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _pressureTemp = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _vorticity = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);
    _divergence = CreateRWTexture2D(DXGI_FORMAT_R16_FLOAT);

    // R16G16B16A16 FLOAT
    _density = CreateRWTexture2D(DXGI_FORMAT_R16G16B16A16_FLOAT);
    _densityTemp = CreateRWTexture2D(DXGI_FORMAT_R16G16B16A16_FLOAT);

    // R32 SINT
    // Boundary Condition
    // 0: Normal Cell
    // 1: Dirichlet Boundary Condition 
    // 2: Neumann Boundary Condition
    // 3: Periodic Boundary Condition (Wrap-around)
    _boundaryCondition = CreateRWTexture2D(DXGI_FORMAT_R32_SINT);
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
    // ImGui::Checkbox()
    // ImGui::End();

    ImGui::Begin("Stable Fluids");
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    ImGui::InputFloat("Vorticity Scale", &_vorticityScale, 1.0, 1.0, "%.1f");
    ImGui::End();
}

void StableFluids::UpdateSimulationParams() {
    static int color = 0;
    static bool prevLeftButton = false;
    static Vec2 prevMouseNdc = Vec2(-1.0f, -1.0f);

    _stableFluidsParams.dt = DELTA_TIME;
    _stableFluidsParams.viscosity = _viscosity;
    _stableFluidsParams.vorticityScale = _vorticityScale;
    _stableFluidsParams.wallBoundaryCondition = _wallBoundaryCondition;

    if (INPUT->GetButton(KEY_TYPE::LBUTTON)) {
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

            _stableFluidsParams.sourcingDensity = rainbow[(color++) % 7];
            _stableFluidsParams.sourcingVelocity = Vec2(0.0f);
        } else {
            Vec2 ndcVel = INPUT->GetMouseNdc() - prevMouseNdc;
            ndcVel.y = -ndcVel.y; // y축 반전
            _stableFluidsParams.sourcingVelocity = ndcVel * 10.0f;
        }
    } else {
        _stableFluidsParams.i = -1; // uint의 Overflow 이용(좌클릭을 누르지 않을때 CS에서 처리)
    }

    prevLeftButton = INPUT->GetButton(KEY_TYPE::LBUTTON);
    prevMouseNdc = INPUT->GetMouseNdc();

    _stableFluidsParamsCB->UpdateData(&_stableFluidsParams, sizeof(_stableFluidsParams));
}

void StableFluids::Sourcing() {
    // Sourcing
    _stableFluidsParamsCB->BindToCompute(CBV_REGISTER::b0);

    _velocity->BindUAVToCompute(UAV_REGISTER::u0);
    _density->BindUAVToCompute(UAV_REGISTER::u1);
    _boundaryCondition->BindUAVToCompute(UAV_REGISTER::u2);

    GEngine->GetComputeDescHeap()->CommitTable();

    _sourcingCS->Update();

    COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)), 1);

    D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get()),
                                            CD3DX12_RESOURCE_BARRIER::UAV(_density->GetTex2D().Get()),
                                            CD3DX12_RESOURCE_BARRIER::UAV(_boundaryCondition->GetTex2D().Get())
    };

    COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
}

void StableFluids::ComputeVorticity() {
    // Vorticity calculation
    _velocity->BindSRVToCompute(SRV_REGISTER::t0);
    _vorticity->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    _computeVorticityCS->Update();

    COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)), 1);

    D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_vorticity->GetTex2D().Get())};

    COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
}

void StableFluids::ConfineVorticity() {
    _stableFluidsParamsCB->BindToCompute(CBV_REGISTER::b0);

    // Vorticity confinemenet
    _vorticity->BindSRVToCompute(SRV_REGISTER::t0);
    _velocity->BindUAVToCompute(UAV_REGISTER::u0);

    GEngine->GetComputeDescHeap()->CommitTable();

    _confineVorticityCS->Update();

    COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)), 1);

    D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get())};

    COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
}

void StableFluids::Diffuse() {
    for (int32 i = 0; i < 10; i++) {
        _stableFluidsParamsCB->BindToCompute(CBV_REGISTER::b0);

        if (i % 2 == 0) {
            _velocity->BindSRVToCompute(SRV_REGISTER::t0);
            _density->BindSRVToCompute(SRV_REGISTER::t1);

            _velocityTemp->BindUAVToCompute(UAV_REGISTER::u0);
            _densityTemp->BindUAVToCompute(UAV_REGISTER::u1);
        } else {
            _velocityTemp->BindSRVToCompute(SRV_REGISTER::t0);
            _densityTemp->BindSRVToCompute(SRV_REGISTER::t1);

            _velocity->BindUAVToCompute(UAV_REGISTER::u0);
            _density->BindUAVToCompute(UAV_REGISTER::u1);
        }

        GEngine->GetComputeDescHeap()->CommitTable();

        _diffuseCS->Update();

        COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)),
                                   1);

        if (i % 2 == 0) {
            D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocityTemp->GetTex2D().Get()),
                                                    CD3DX12_RESOURCE_BARRIER::UAV(_densityTemp->GetTex2D().Get())};
            COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
        } else {
            D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get()),
                                                    CD3DX12_RESOURCE_BARRIER::UAV(_density->GetTex2D().Get())};
            COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
        }
    }
}

void StableFluids::Projection() {

    // Compute divergence

    {
        _velocity->BindSRVToCompute(SRV_REGISTER::t0);
        _divergence->BindUAVToCompute(UAV_REGISTER::u0);
        _pressure->BindUAVToCompute(UAV_REGISTER::u1);
        _pressureTemp->BindUAVToCompute(UAV_REGISTER::u2);

        GEngine->GetComputeDescHeap()->CommitTable();

        _divergenceCS->Update();

        COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)),
                                   1);

        D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_divergence->GetTex2D().Get()),
                                                CD3DX12_RESOURCE_BARRIER::UAV(_pressure->GetTex2D().Get()),
                                                CD3DX12_RESOURCE_BARRIER::UAV(_pressureTemp->GetTex2D().Get())};

        COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
    }

    // Jacobi iteration

    {
        _jacobiCS->Update(); // 한 번만 Shader Update

        for (int32 i = 0; i < 100; i++) {
            if (i % 2 == 0) {
                _pressure->BindSRVToCompute(SRV_REGISTER::t0);
                _divergence->BindSRVToCompute(SRV_REGISTER::t1);
                _pressureTemp->BindUAVToCompute(UAV_REGISTER::u0);
            } else {
                _pressureTemp->BindSRVToCompute(SRV_REGISTER::t0);
                _divergence->BindSRVToCompute(SRV_REGISTER::t1);
                _pressure->BindUAVToCompute(UAV_REGISTER::u0);
            }
            GEngine->GetComputeDescHeap()->CommitTable();

            COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)),
                                       static_cast<UINT>(ceil(_height / 32.0f)), 1);

            if (i % 2 == 0) {
                D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_pressureTemp->GetTex2D().Get())};
                COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
            } else {
                D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_pressure->GetTex2D().Get())};
                COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
            }
        }
    }

    // Apply pressure

    {
        _pressure->BindSRVToCompute(SRV_REGISTER::t0);
        _velocity->BindUAVToCompute(UAV_REGISTER::u0);

        GEngine->GetComputeDescHeap()->CommitTable();

        _applyPressureCS->Update();

        COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)),
                                   1);

        D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get())};
        COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
    }
}

void StableFluids::Advection() {
    _velocityTemp->CopyResource(_velocity);
    _densityTemp->CopyResource(_density);

    _stableFluidsParamsCB->BindToCompute(CBV_REGISTER::b0);

    _velocityTemp->BindSRVToCompute(SRV_REGISTER::t0);
    _densityTemp->BindSRVToCompute(SRV_REGISTER::t1);

    _velocity->BindUAVToCompute(UAV_REGISTER::u0);
    _density->BindUAVToCompute(UAV_REGISTER::u1);

    GEngine->GetComputeDescHeap()->CommitTable();

    _advectionCS->Update();

    COMPUTE_CMD_LIST->Dispatch(static_cast<UINT>(ceil(_width / 32.0f)), static_cast<UINT>(ceil(_height / 32.0f)), 1);

    D3D12_RESOURCE_BARRIER uavBarriers[] = {CD3DX12_RESOURCE_BARRIER::UAV(_velocity->GetTex2D().Get()),
                                            CD3DX12_RESOURCE_BARRIER::UAV(_density->GetTex2D().Get())};
    COMPUTE_CMD_LIST->ResourceBarrier(_countof(uavBarriers), uavBarriers);
}
