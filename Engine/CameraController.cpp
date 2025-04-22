#include "pch.h"
#include "CameraController.h"
#include "Transform.h"
#include "Camera.h"
#include "Input.h"
#include "Timer.h"

CameraController::CameraController() {}

CameraController::~CameraController() {}

void CameraController::LateUpdate() {
    if (INPUT->GetButtonDown(KEY_TYPE::F))
        _useFirstPersonView = !_useFirstPersonView;

    if (!_useFirstPersonView)
        return;

    if (INPUT->GetButtonDown(KEY_TYPE::SHIFT)) {
        _speed = 2.0f;
        GetCamera()->setFOV(XM_2PI / 5.05f); // �޸��� FOV ���
    }
    if (INPUT->GetButtonUp(KEY_TYPE::SHIFT)) {
        _speed = 1.0f;
        GetCamera()->setFOV(XM_2PI / 5.0f);
    }

    // �밢�ӵ� ���� �ʿ�
    if (INPUT->GetButton(KEY_TYPE::W))
        MoveForward(DELTA_TIME);
    if (INPUT->GetButton(KEY_TYPE::S))
        MoveForward(-DELTA_TIME);
    if (INPUT->GetButton(KEY_TYPE::D))
        MoveRight(DELTA_TIME);
    if (INPUT->GetButton(KEY_TYPE::A))
        MoveRight(-DELTA_TIME);
    OnMouseMove();
}

void CameraController::OnMouseMove() {
    Vec2 currentMouseNdc = INPUT->GetMouseNdc();

    // ���� �������� ���콺 ��ġ�� ���Ͽ� �̵���(Delta) ���
    Vec2 deltaMouse = currentMouseNdc - _lastMouseNdc;
    _lastMouseNdc = currentMouseNdc; // ���� ��ġ�� ���� �����ӿ��� ����� ���� ��ġ�� ����

    // ���콺 �̵����� �����Ͽ� ȸ�� ����
    _yaw += deltaMouse.x * XM_2PI;      // �¿� ȸ�� (Y�� ����)
    _pitch -= deltaMouse.y * XM_PIDIV2; // ���� ȸ�� (X�� ����)

    // Pitch �� ���� (-90�� ~ +90��)
    _pitch = std::clamp(_pitch, -XM_PIDIV2, XM_PIDIV2); // Ndc clamp���� �ɷ����� ��.

    // ���ʹϾ��� �̿��Ͽ� ȸ�� ����
    Quaternion rotation = Quaternion::CreateFromYawPitchRoll(_yaw, _pitch, 0.0f);
    GetTransform()->SetLocalRotation(rotation);
}

void CameraController::MoveForward(float dt) {
    Vec3 forward = GetTransform()->GetForward();
    GetTransform()->Translate(forward * _speed * dt);
}

void CameraController::MoveRight(float dt) {
    Vec3 right = GetTransform()->GetRight();
    GetTransform()->Translate(right * _speed * dt);
}
