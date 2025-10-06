#include "pch.h"
#include "Transform.h"
#include "CBufferStructs.h"
#include "Engine.h"
#include "ConstantBuffer.h"

Transform::Transform() : Component(COMPONENT_TYPE::TRANSFORM) {}

Transform::~Transform() {}

void Transform::FinalUpdate() {
    Matrix matScale = Matrix::CreateScale(_localScale);

    // Quaternion
    // if (_axis.Length() > FLT_EPSILON)
    //	_axis.Normalize();
    // else
    //	_axis = Vec3(0.0f, 1.0f, 0.0f); // �⺻�� ����

    // SimpleMath::Quaternion q = SimpleMath::Quaternion(_axis * sin(_angle * 0.5f), cos(_angle * 0.5f));
    //_localRotation = SimpleMath::Quaternion::CreateFromAxisAngle(_axis, _angle);

    Matrix matRotation = Matrix::CreateFromQuaternion(_localRotation);
    Matrix matTranslation = Matrix::CreateTranslation(_localPosition);

    _matLocal = matScale * matRotation * matTranslation;
    _matWorld = _matLocal; // temp ���߿� �θ� ��ӵǴ� local

    // InvTranspose
    _matInvTranspose = _matWorld;
    _matInvTranspose.Translation(Vec3(0.0f));
    _matInvTranspose = _matInvTranspose.Invert().Transpose();

    // ���� �θ� �ִٸ� �θ� �������� ����
    // shared_ptr<Transform> parent = GetParent().lock();
    // if (parent != nullptr)
    //{
    //	_matWorld *= parent->GetLocalToWorldMatrix();
    //}
}

void Transform::PushData() {
    TransformParams transformParams = {};
    transformParams.matWorld = _matWorld.Transpose();
    transformParams.matInvTranspose = _matInvTranspose.Transpose();

    shared_ptr<ConstantBuffer> transformParamsCB = GEngine->GetTransformParamsCB();
    transformParamsCB->UpdateData(&transformParams, sizeof(transformParams));
    //transformParamsCB->SetGraphicsRootCBV(CBV_REGISTER::b1); // ����
    transformParamsCB->BindToGraphics(CBV_REGISTER::b1);
}
