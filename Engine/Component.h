#pragma once

#include "Object.h"

enum class COMPONENT_TYPE : uint8 {
    TRANSFORM,
    MESH_RENDERER,
    CAMERA,

    // ...
    MONO_BEHAVIOUR,
    END,
};

enum { FIXED_COMPONENT_COUNT = static_cast<uint8>(COMPONENT_TYPE::END) - 1 };

class SimulationObject;
class Transform;
class MeshRenderer;
class Camera;
class CameraController;

class Component : public Object {
  public:
    Component(COMPONENT_TYPE type);
    virtual ~Component();

  public:
    virtual void Awake() {}
    virtual void Start() {}
    virtual void Update() {}
    virtual void LateUpdate() {}
    virtual void FinalUpdate() {}

  public:
    COMPONENT_TYPE GetType() { return _type; }

    shared_ptr<SimulationObject> GetSimulationObject();
    shared_ptr<Transform> GetTransform();
    shared_ptr<MeshRenderer> GetMeshRenderer();
    shared_ptr<Camera> GetCamera();
    shared_ptr<CameraController> GetCameraController();

  private:
    friend class SimulationObject;
    void SetSimulationObject(shared_ptr<SimulationObject> simulationObject) { _simulationObject = simulationObject; }

  protected:
    COMPONENT_TYPE _type;
    weak_ptr<SimulationObject> _simulationObject;
};
