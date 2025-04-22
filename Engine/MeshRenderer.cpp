#include "pch.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

MeshRenderer::MeshRenderer() : Component(COMPONENT_TYPE::MESH_RENDERER) {}

MeshRenderer::~MeshRenderer() {}

void MeshRenderer::Render() {
    for (uint32 i = 0; i < _materials.size(); i++) {
        shared_ptr<GraphicsMaterial> &material = _materials[i];

        if (material == nullptr || material->GetShader() == nullptr)
            continue;

        GetTransform()->PushData();

        material->PushData();
        _mesh->Render(1, i);
    }
}

void MeshRenderer::SetMaterial(shared_ptr<GraphicsMaterial> material, uint32 idx) {
    if (_materials.size() <= static_cast<size_t>(idx))
        _materials.resize(static_cast<size_t>(idx + 1));

    _materials[idx] = material;
}
