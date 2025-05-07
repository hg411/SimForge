#include "pch.h"
#include "StableFluids.h"
#include "SimulationObject.h"
#include "Transform.h"
#include "Camera.h"

StableFluids::StableFluids() {}

StableFluids::~StableFluids() {}

void StableFluids::Init() {
    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitStructuredBuffers();
    InitSimulationObjects();
}

void StableFluids::Update() {}

void StableFluids::FinalUpdate() {}

void StableFluids::Render() {}

void StableFluids::InitShaders() {}

void StableFluids::InitConstantBuffers() {}

void StableFluids::InitTextures() {}

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
