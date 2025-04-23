#include "pch.h"
#include "Camera.h"
#include "Engine.h"
#include "Transform.h"
#include "CBufferStructs.h"

Camera::Camera() : Component(COMPONENT_TYPE::CAMERA) {
    _width = static_cast<float>(GEngine->GetWindowInfo().width);
    _height = static_cast<float>(GEngine->GetWindowInfo().height);
}

Camera::~Camera() {}

void Camera::FinalUpdate() {
    // �ػ� ������ ���. �� ȿ������ ������� �ٲ� �ʿ� �־��.
    _width = static_cast<float>(GEngine->GetWindowInfo().width);
    _height = static_cast<float>(GEngine->GetWindowInfo().height);
    float aspectRatio = _width / _height;

    Matrix rotationMatrix =
        XMMatrixRotationQuaternion(GetTransform()->GetLocalRotation()); // Quaternion���κ��� Rotation ��� ����

    _matView = ::XMMatrixLookToLH(GetTransform()->GetWorldPosition(), GetTransform()->GetForward(), GetTransform()->GetUp());

    if (_type == PROJECTION_TYPE::PERSPECTIVE) {
        _matProjection = XMMatrixPerspectiveFovLH(_fov, aspectRatio, _near, _far);
    } else {
        //_matProjection =
        //    ::XMMatrixOrthographicOffCenterLH(-aspectRatio, aspectRatio, -1.0f, 1.0f, _far, _near);
        _matProjection = ::XMMatrixOrthographicLH(2.0f * aspectRatio, 2.0f, _near, _far);
    }

    _matVP = _matView * _matProjection;
    _matInvProj = _matProjection.Invert();
}

void Camera::PushData(GlobalParams &globalParams) {
    globalParams.matView = _matView.Transpose();
    globalParams.matProj = _matProjection.Transpose();
    globalParams.matVP = GetViewProjMatrix().Transpose();
    globalParams.matInvProj = GetInverseProjMatrix().Transpose();
    globalParams.eyePos = GetTransform()->GetWorldPosition();
    globalParams.camUp = GetTransform()->GetUp();
    globalParams.camRight = GetTransform()->GetRight();
}
