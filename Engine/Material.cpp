#include "pch.h"
#include "Material.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "TableDescriptorHeap.h"
#include "Texture.h"

GraphicsMaterial::GraphicsMaterial() : Object(OBJECT_TYPE::GRAPHICS_MATERIAL) {}

GraphicsMaterial::~GraphicsMaterial() {}

void GraphicsMaterial::PushData() {
    // CBV ���ε�
    GEngine->GetMaterialParamsCB()->PushGraphicsData(&_params, sizeof(_params));

    // SRV ���ε�
    for (size_t i = 0; i < _textures.size(); i++) {
        if (_textures[i] == nullptr)
            continue;

        SRV_REGISTER reg = SRV_REGISTER(static_cast<int8>(SRV_REGISTER::t0) + i);
        GEngine->GetGraphicsDescHeap()->SetSRV(_textures[i]->GetSRVHandle(), reg);
    }

    // ���������� ����
    _shader->Update();
}
