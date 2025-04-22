#pragma once

#include "Component.h"
#include "Object.h"

class Transform;
class MeshRenderer;
class Camera;
class MonoBehaviour;

// class Light;
// class Skybox;
// class Collider;

// endable_shared_from_this : this 포인터를 스마트 포인터로 활용
class SimulationObject : public Object, public enable_shared_from_this<SimulationObject> {
  public:
    SimulationObject();
    virtual ~SimulationObject();

    void Awake();
    void Start();
    void Update();
    void LateUpdate();
    void FinalUpdate();

    shared_ptr<Component> GetFixedComponent(COMPONENT_TYPE type);

    shared_ptr<Transform> GetTransform();
    shared_ptr<MeshRenderer> GetMeshRenderer();
    shared_ptr<Camera> GetCamera();
    shared_ptr<CameraController> GetCameraController();
    //shared_ptr<Light> GetLight();
    //shared_ptr<Skybox> GetSkybox();
    //shared_ptr<Collider> GetCollider();

    void AddComponent(shared_ptr<Component> component);

  private:
    array<shared_ptr<Component>, FIXED_COMPONENT_COUNT> _components;
    vector<shared_ptr<MonoBehaviour>> _scripts;
};
