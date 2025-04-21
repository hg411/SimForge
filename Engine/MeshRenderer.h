#pragma once

#include "Component.h"

class Mesh;
class Material;

class MeshRenderer : public Component {
  public:
    MeshRenderer();
    virtual ~MeshRenderer();

    void Render();

  public:
    shared_ptr<Mesh> GetMesh() { return _mesh; }
    shared_ptr<GraphicsMaterial> GetMaterial(uint32 idx = 0) { return _materials[idx]; }

    void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
    void SetMaterial(shared_ptr<GraphicsMaterial> material, uint32 idx);

  private:
    shared_ptr<Mesh> _mesh;
    vector<shared_ptr<GraphicsMaterial>> _materials;
};
