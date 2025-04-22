#include "pch.h"
#include "Simulation.h"
#include "SimulationObject.h"
#include "CBufferStructs.h"
#include "Camera.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "SwapChain.h"
#include "RenderTargetGroup.h"
#include "MeshRenderer.h"
#include "Imgui.h"

Simulation::Simulation() {}

Simulation::~Simulation() {}

void Simulation::Init() {
    InitImgui();
    InitShaders();
    InitSimulationObjects();
}

void Simulation::Awake() {
    for (const shared_ptr<SimulationObject> &simulationObject : _simulationObjects) {
        simulationObject->Awake();
    }
}

void Simulation::Start() {
    for (const shared_ptr<SimulationObject> &simulationObject : _simulationObjects) {
        simulationObject->Start();
    }
}

void Simulation::Update() {
    _imgui->Update(shared_from_this());

    for (const shared_ptr<SimulationObject> &simulationObject : _simulationObjects) {
        simulationObject->Update();
    }
}

void Simulation::LateUpdate() {
    for (const shared_ptr<SimulationObject> &simulationObject : _simulationObjects) {
        simulationObject->LateUpdate();
    }
}

void Simulation::FinalUpdate() {
    for (const shared_ptr<SimulationObject> &simulationObject : _simulationObjects) {
        simulationObject->FinalUpdate();
    }
}

shared_ptr<Camera> Simulation::GetMainCamera() {
    if (_cameras.empty())
        return nullptr;

    return _cameras[0];
}

void Simulation::Render() {
    PushGlobalData();
    ClearRTV();

    int8 backIndex = GEngine->GetSwapChain()->GetBackBufferIndex();
    GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->OMSetRenderTargets(1, backIndex);

    for (auto &simulationObject : _simulationObjects) {
        shared_ptr<MeshRenderer> meshRenderer = simulationObject->GetMeshRenderer();
        if (meshRenderer) {
            meshRenderer->Render();
        }
    }
}

void Simulation::PushGlobalData() {
    GlobalParams globalParams = {};

    shared_ptr<Camera> mainCamera = GetMainCamera();
    assert(mainCamera && "Main camera is not set!");

    mainCamera->PushData(globalParams);

    GEngine->GetGlobalParamsCB()->SetGraphicsGlobalData(&globalParams, sizeof(globalParams));
}

void Simulation::ClearRTV() {
    // SwapChain Group ÃÊ±âÈ­
    int8 backIndex = GEngine->GetSwapChain()->GetBackBufferIndex();
    GEngine->GetRTGroup(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)->ClearRenderTargetView(backIndex);
}

void Simulation::InitImgui() {
    _imgui = make_shared<Imgui>();
    _imgui->Init();
}

void Simulation::AddSimulationObject(shared_ptr<SimulationObject> simulationObject) {
    if (simulationObject->GetCamera() != nullptr) {
        _cameras.push_back(simulationObject->GetCamera());
    }

    _simulationObjects.push_back(simulationObject);
}

void Simulation::AddCamera(shared_ptr<Camera> camera) { _cameras.push_back(camera); }
