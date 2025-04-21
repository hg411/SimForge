#pragma once

#include "Object.h"
#include "CBufferStructs.h"

enum {
    MATERIAL_ARG_COUNT = 4,
};

class Shader;
class Texture;

class GraphicsMaterial : public Object {
  public:
    GraphicsMaterial();
    virtual ~GraphicsMaterial();

    void PushData();

  public:
    shared_ptr<Shader> GetShader() { return _shader; }

    void SetShader(shared_ptr<Shader> shader) { _shader = shader; }
    void SetTexture(uint8 index, shared_ptr<Texture> texture) {
        _textures[index] = texture;
        //_params.SetTexOn(index, (texture == nullptr ? 0 : 1));
    }
    void SetAlbedo(const Vec3 &albedo) { _materialParams.albedo = albedo; }
    void SetMetallic(const float metallic) { _materialParams.metallic = metallic; }
    void SetRoughness(const float roughness) { _materialParams.roughness = roughness; }

  private:
    shared_ptr<Shader> _shader;
    MaterialParams _materialParams;
    array<shared_ptr<Texture>, MATERIAL_ARG_COUNT> _textures;
};

class ComputeMaterial : public Object {
  public:
    ComputeMaterial();
    virtual ~ComputeMaterial();

    void PushData();

  public:
    void SetSRV(SRV_REGISTER reg, shared_ptr<Texture> texture);
    void SetUAV(UAV_REGISTER reg, shared_ptr<Texture> texture);

  private:
    shared_ptr<Shader> _shader;
    array<shared_ptr<Texture>, SRV_REGISTER_COUNT> _srvs;
    array<shared_ptr<Texture>, UAV_REGISTER_COUNT> _uavs;
};