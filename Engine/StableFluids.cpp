#include "pch.h"
#include "StableFluids.h"
#include "Engine.h"
#include "SimulationObject.h"
#include "Transform.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"
#include "ConstantBuffer.h"

StableFluids::StableFluids() {}

StableFluids::~StableFluids() {}

void StableFluids::Init() {
    const WindowInfo &windowInfo = GEngine->GetWindowInfo();
    _width = windowInfo.width;
    _height = windowInfo.height;

    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitTextures();
    InitSimulationObjects();
}

void StableFluids::Update() {}

void StableFluids::FinalUpdate() {}

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
    _constantBuffer = make_shared<ConstantBuffer>();
    _constantBuffer->Init(sizeof(StableFluidsParams), 1);

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
    ImGui::Begin("Color");
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);


}