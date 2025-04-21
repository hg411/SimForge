#pragma once

#include "pch.h"

struct GlobalParams {
    Matrix matView;
    Matrix matProj;
    Matrix matVP;
    Matrix matInvProj;
    Vec3 eyePos;
    float padding1;

    Vec3 camUp;
    float padding2;

    Vec3 camRight;
    float padding3;
};

struct TransformParams {
    Matrix matWorld;
    Matrix matInvTranspose;
};

struct MaterialParams {
    Vec3 albedo = Vec3(1.0f);
    float metallic = 0.0f;
    float roughness = 0.0f;
    Vec3 padding;
};