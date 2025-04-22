#include "pch.h"
#include "SimulationObject.h"
#include "Monobehaviour.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Camera.h"
#include "CameraController.h"



SimulationObject::SimulationObject() : Object(OBJECT_TYPE::SIMULATION_OBJECT) {}

SimulationObject::~SimulationObject() {}

void SimulationObject::Awake() {
    for (shared_ptr<Component> &component : _components) {
        if (component)
            component->Awake();
    }

    for (shared_ptr<MonoBehaviour> &script : _scripts) {
        script->Awake();
    }
}

void SimulationObject::Start() {
    for (shared_ptr<Component> &component : _components) {
        if (component)
            component->Start();
    }

    for (shared_ptr<MonoBehaviour> &script : _scripts) {
        script->Start();
    }
}

void SimulationObject::Update() {
    for (shared_ptr<Component> &component : _components) {
        if (component)
            component->Update();
    }

    for (shared_ptr<MonoBehaviour> &script : _scripts) {
        script->Update();
    }
}

void SimulationObject::LateUpdate() {
    for (shared_ptr<Component> &component : _components) {
        if (component)
            component->LateUpdate();
    }

    for (shared_ptr<MonoBehaviour> &script : _scripts) {
        script->LateUpdate();
    }
}

void SimulationObject::FinalUpdate() {
    for (shared_ptr<Component> &component : _components) {
        if (component)
            component->FinalUpdate();
    }
}

shared_ptr<Component> SimulationObject::GetFixedComponent(COMPONENT_TYPE type) {
    uint8 index = static_cast<uint8>(type);
    assert(index < FIXED_COMPONENT_COUNT);
    return _components[index];
}

shared_ptr<Transform> SimulationObject::GetTransform() {
    shared_ptr<Component> component = GetFixedComponent(COMPONENT_TYPE::TRANSFORM);
    return static_pointer_cast<Transform>(component);
}

shared_ptr<MeshRenderer> SimulationObject::GetMeshRenderer() {
    shared_ptr<Component> component = GetFixedComponent(COMPONENT_TYPE::MESH_RENDERER);
    return static_pointer_cast<MeshRenderer>(component);
}

shared_ptr<Camera> SimulationObject::GetCamera() {
    shared_ptr<Component> component = GetFixedComponent(COMPONENT_TYPE::CAMERA);
    return static_pointer_cast<Camera>(component);
}

shared_ptr<CameraController> SimulationObject::GetCameraController() {
    for (shared_ptr<MonoBehaviour> &script : _scripts) {
        shared_ptr<CameraController> cameraController = dynamic_pointer_cast<CameraController>(script);
        if (cameraController) {
            return cameraController;
        }
    }

    return nullptr;
}

void SimulationObject::AddComponent(shared_ptr<Component> component) {
    component->SetSimulationObject(shared_from_this());

    uint8 index = static_cast<uint8>(component->GetType());
    if (index < FIXED_COMPONENT_COUNT) {
        _components[index] = component;
    } else {
        _scripts.push_back(dynamic_pointer_cast<MonoBehaviour>(component));
    }
}
