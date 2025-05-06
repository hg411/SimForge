#include "pch.h"
#include "StableFluid.h"
#include "SimulationObject.h"
#include "Transform.h"

StableFluid::StableFluid() {}

StableFluid::~StableFluid() {}

void StableFluid::Init() {
    InitImgui();
    InitShaders();
    InitConstantBuffers();
    InitStructuredBuffers();
    InitSimulationObjects();
}

void StableFluid::Update() {}

void StableFluid::FinalUpdate() {}

void StableFluid::Render() {}

void StableFluid::InitShaders() {}

void StableFluid::InitConstantBuffers() {}

void StableFluid::InitStructuredBuffers() {}

void StableFluid::InitSimulationObjects() {
    // Main Camera
    {
        shared_ptr<SimulationObject> obj = make_shared<SimulationObject>();
        obj->SetName(L"Main_Camera");

        shared_ptr<Transform> trasnform = make_shared<Transform>();


    }
}

void StableFluid::BuildUI() {}
