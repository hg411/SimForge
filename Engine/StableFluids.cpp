#include "pch.h"
#include "StableFluids.h"
#include "SimulationObject.h"
#include "Transform.h"
#include "Camera.h"
#include "Texture.h"

StableFluids::StableFluids() {}

StableFluids::~StableFluids() {}

void StableFluids::Init() {
    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitTextures();
    InitSimulationObjects();
}

void StableFluids::Update() {}

void StableFluids::FinalUpdate() {}

void StableFluids::Render() {}

void StableFluids::InitShaders() {}

void StableFluids::InitConstantBuffers() {}

void StableFluids::InitTextures() {
    _velocity = make_shared<Texture>();
    _velocity->Create(DXGI_FORMAT_R16G16_FLOAT, _width, _height, CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                      D3D12_HEAP_FLAG_NONE,
                      D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, // UAV 접근 가능해야 함
                      Vec4(0, 0, 0, 0)                            // 필요 시 clearColor)
    );
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
