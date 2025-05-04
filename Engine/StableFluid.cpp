#include "pch.h"
#include "StableFluid.h"

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

void StableFluid::InitSimulationObjects() {}

void StableFluid::BuildUI() {}
