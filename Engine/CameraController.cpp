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
        GetCamera()->setFOV(XM_2PI / 5.05f); // 달릴때 FOV 축소
    }
    if (INPUT->GetButtonUp(KEY_TYPE::SHIFT)) {
        _speed = 1.0f;
        GetCamera()->setFOV(XM_2PI / 5.0f);
    }

    // 대각속도 변경 필요
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

    // 이전 프레임의 마우스 위치와 비교하여 이동량(Delta) 계산
    Vec2 deltaMouse = currentMouseNdc - _lastMouseNdc;
    _lastMouseNdc = currentMouseNdc; // 현재 위치를 다음 프레임에서 사용할 이전 위치로 저장

    // 마우스 이동량을 누적하여 회전 적용
    _yaw += deltaMouse.x * XM_2PI;      // 좌우 회전 (Y축 기준)
    _pitch -= deltaMouse.y * XM_PIDIV2; // 상하 회전 (X축 기준)

    // Pitch 값 제한 (-90도 ~ +90도)
    _pitch = std::clamp(_pitch, -XM_PIDIV2, XM_PIDIV2); // Ndc clamp에서 걸러지긴 함.

    // 쿼터니언을 이용하여 회전 적용
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
