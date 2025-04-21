#include "pch.h"
#include "Component.h"
#include "SimulationObject.h"

Component::Component(COMPONENT_TYPE type) : Object(OBJECT_TYPE::COMPONENT), _type(type) {}

Component::~Component() {}

shared_ptr<SimulationObject> Component::GetSimulationObject() { return _simulationObject.lock(); }

shared_ptr<Transform> Component::GetTransform() {
    // lock std::weak_ptr이 소유한 객체의 std::shared_ptr을 안전하게 얻는 데 사용
    return _simulationObject.lock()->GetTransform();
}

shared_ptr<MeshRenderer> Component::GetMeshRenderer() { return _simulationObject.lock()->GetMeshRenderer(); }

shared_ptr<Camera> Component::GetCamera() { return _simulationObject.lock()->GetCamera(); }

shared_ptr<CameraController> Component::GetCameraController() {
    return _simulationObject.lock()->GetCameraController();
}
