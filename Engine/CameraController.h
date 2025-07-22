#pragma once

#include "MonoBehaviour.h"

class Camera;

class CameraController : public MonoBehaviour {
  public:
    CameraController();
    virtual ~CameraController();

    void BuildUI();
    void LateUpdate() override;

    bool *GetUseFPVPtr() { return &_useFirstPersonView; }

  private:
    void OnMouseMove();
    void MoveForward(float dt);
    void MoveRight(float dt);

  private:
    float _speed = 1.0f;
    float _pitch = 0.0f;
    float _yaw = 0.0f;
    Vec2 _lastMouseNdc = {0.0f, 0.0f};

    bool _useFirstPersonView = false;
};
