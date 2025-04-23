#pragma once

#include "Component.h"

struct GlobalParams;

enum class PROJECTION_TYPE {
    PERSPECTIVE,  // 원근 투영
    ORTHOGRAPHIC, // 직교 투영
};

class Camera : public Component {
  public:
    Camera();
    virtual ~Camera();

    void FinalUpdate() override;
    void PushData(GlobalParams &globalParams);

  public:


  public:
    PROJECTION_TYPE GetProjectionType() { return _type; }
    const Matrix &GetViewMatrix() const { return _matView; }
    const Matrix &GetProjectionMatrix() const { return _matProjection; }
    const Matrix &GetViewProjMatrix() const { return _matVP; }
    const Matrix &GetInverseProjMatrix() const { return _matInvProj; }

    void SetProjectionType(PROJECTION_TYPE type) { _type = type; }
    void SetNear(float value) { _near = value; }
    void SetFar(float value) { _far = value; }
    void setFOV(float value) { _fov = value; }
    void SetScale(float value) { _scale = value; }
    void SetWidth(float value) { _width = value; }
    void SetHeight(float value) { _height = value; }

  private:
    PROJECTION_TYPE _type = PROJECTION_TYPE::PERSPECTIVE;

    float _near = 0.01f;
    float _far = 100.0f;
    float _fov = XM_2PI / 5.0f; // field of view
    float _scale = 1.f;
    float _width = 0.0f;
    float _height = 0.0f;

    Matrix _matView = {};
    Matrix _matProjection = {};
    Matrix _matVP = {};
    Matrix _matInvProj = {};
};
