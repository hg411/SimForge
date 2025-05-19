#include "pch.h"
#include "StableFluids.h"
#include "Engine.h"
#include "SimulationObject.h"
#include "Transform.h"
#include "Camera.h"
#include "Texture.h"

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



}

void StableFluids::InitConstantBuffers() {}

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

void StableFluids::BuildUI() {}