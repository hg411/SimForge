#include "pch.h"
#include "Material.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "TableDescriptorHeap.h"
#include "Texture.h"
#include "Shader.h"

GraphicsMaterial::GraphicsMaterial() : Object(OBJECT_TYPE::GRAPHICS_MATERIAL) {}

GraphicsMaterial::~GraphicsMaterial() {}

void GraphicsMaterial::PushData() {
    // CBV 업로드
    GEngine->GetMaterialParamsCB()->PushGraphicsData(&_materialParams, sizeof(_materialParams));

    // SRV 업로드
    for (size_t i = 0; i < _textures.size(); i++) {
        if (_textures[i] == nullptr)
            continue;

        SRV_REGISTER reg = SRV_REGISTER(static_cast<int8>(SRV_REGISTER::t0) + i);
        GEngine->GetGraphicsDescHeap()->SetSRV(_textures[i]->GetSRVHandle(), reg);
    }

    // 파이프라인 세팅
    _shader->Update();
}

ComputeMaterial::ComputeMaterial() : Object(OBJECT_TYPE::COMPUTE_MATERIAL) {}

ComputeMaterial::~ComputeMaterial() {}

void ComputeMaterial::PushData() {
    auto computeHeap = GEngine->GetComputeDescHeap();

    for (uint8 i = 0; i < _srvs.size(); ++i) {
        if (_srvs[i]) {
            SRV_REGISTER reg = static_cast<SRV_REGISTER>(static_cast<uint8>(SRV_REGISTER::t0) + i);
            computeHeap->SetSRV(_srvs[i]->GetSRVHandle(), reg);
        }
    }

    for (uint8 i = 0; i < _uavs.size(); ++i) {
        if (_uavs[i]) {
            UAV_REGISTER reg = static_cast<UAV_REGISTER>(static_cast<uint8>(UAV_REGISTER::u0) + i);
            computeHeap->SetUAV(_uavs[i]->GetUAVHandle(), reg);
        }
    }

    _shader->Update();
}

void ComputeMaterial::SetSRV(SRV_REGISTER reg, shared_ptr<Texture> texture) {
    uint8 idx = static_cast<uint8>(reg) - static_cast<uint8>(SRV_REGISTER::t0);
    _srvs[idx] = texture;
}

void ComputeMaterial::SetUAV(UAV_REGISTER reg, shared_ptr<Texture> texture) {

    uint8 idx = static_cast<uint8>(reg) - static_cast<uint8>(UAV_REGISTER::u0);
    _uavs[idx] = texture;
}
