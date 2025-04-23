#pragma once
#include "Component.h"

struct TransformParams;

class Transform : public Component {
  public:
    Transform();
    virtual ~Transform();

    void FinalUpdate() override;
    void PushData();

  public:
    const Vec3 &GetLocalPosition() const { return _localPosition; }
    const Quaternion &GetLocalRotation() const { return _localRotation; }
    const Vec3 &GetLocalScale() const { return _localScale; }

    Vec3 GetWorldPosition() const { return _matWorld.Translation(); }

    const Matrix &GetLocalMatrix() const { return _matLocal; }
    const Matrix &GetWorldMatrix() const { return _matWorld; }
    const Matrix &GetInvTransposeMatrix() const { return _matInvTranspose; }

    Vec3 GetRight() const { return _matWorld.Right(); }
    Vec3 GetUp() const { return _matWorld.Up(); }
    Vec3 GetForward() const { return _matWorld.Backward(); }

    void SetLocalScale(const Vec3 &scale) { _localScale = scale; }
    void SetLocalRotation(const Quaternion &rotation) { _localRotation = rotation; }
    void SetLocalPosition(const Vec3 &position) { _localPosition = position; }

    void RotateWithTrackball(const Quaternion &value) { _localRotation *= value; }
    void Translate(Vec3 offset) { _localPosition += offset; }

  private:
    Vec3 _localPosition = {};
    Quaternion _localRotation = {};
    Vec3 _localScale = Vec3(1.0f);

    // Vec3 _axis = Vec3(0.0f);
    // float _angle = 0.0f;

    Matrix _matLocal = {};
    Matrix _matWorld = {};
    Matrix _matInvTranspose = {};
};
