#include "pch.h"
#include "Input.h"
#include "Engine.h"

void Input::Init(HWND hwnd) {
    _hwnd = hwnd;
    _states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);
}

void Input::Update() {
    HWND hwnd = ::GetActiveWindow();
    if (_hwnd != hwnd) {
        for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
            _states[key] = KEY_STATE::NONE;

        return;
    }

    for (uint32 key = 0; key < KEY_TYPE_COUNT; key++) {
        SHORT keyState = GetAsyncKeyState(key); // 비동기 키 입력 감지 방식

        if (keyState & 0x8000) {
            // 이전 프레임에 키를 누른 상태라면 PRESS
            if (_states[key] == KEY_STATE::NONE || _states[key] == KEY_STATE::UP)
                _states[key] = KEY_STATE::DOWN;
            else
                _states[key] = KEY_STATE::PRESS;
        } else {
            // 이전 프레임에 키를 누른 상태라면 UP
            if (_states[key] == KEY_STATE::PRESS || _states[key] == KEY_STATE::DOWN)
                _states[key] = KEY_STATE::UP;
            else
                _states[key] = KEY_STATE::NONE;
        }
    }

    ::GetCursorPos(&_mousePos);
    ::ScreenToClient(_hwnd, &_mousePos);
}

Vec2 Input::GetMouseNdc() const {
    float mouseXf = static_cast<float>(_mousePos.x);
    float mouseYf = static_cast<float>(_mousePos.y);

    float width = static_cast<float>(GEngine->GetWindowInfo().width);
    float height = static_cast<float>(GEngine->GetWindowInfo().height);

    float mouseNdcX = (mouseXf * 2.0f / width) - 1.0f;
    float mouseNdcY = 1.0f - (mouseYf * 2.0f / height);

    return Vec2(mouseNdcX, mouseNdcY);
}
