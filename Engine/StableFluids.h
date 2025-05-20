#pragma once

#include "Simulation.h"

class Texture;
class Shader;

struct StableFluidsParams {
    float deltaTime;
    float viscosity;
    Vec2 sourcingVelocity;
    Vec4 sourcingDensity;
    uint32 i;
    uint32 j;
};

class StableFluids : public Simulation {
  public:
    StableFluids();
    virtual ~StableFluids();

    void Init() override;

    void Update() override;
    void FinalUpdate() override;
    void Render() override;

  private:
    void InitShaders() override;
    void InitConstantBuffers() override;
    void InitTextures() override;
    void InitSimulationObjects() override;

    void BuildUI() override;

  private:
    uint32 _width = 0;
    uint32 _height = 0;

    // Texture
    shared_ptr<Texture> _velocity;
    shared_ptr<Texture> _velocityTemp;
    shared_ptr<Texture> _pressure;
    shared_ptr<Texture> _pressureTemp;
    shared_ptr<Texture> _density;
    shared_ptr<Texture> _densityTemp;
    shared_ptr<Texture> _vorticity;
    shared_ptr<Texture> _divergence;

    // Compute Shader
    shared_ptr<Shader> _advectionCS;
    shared_ptr<Shader> _applyPressureCS;
    shared_ptr<Shader> _diffuseCS;
    shared_ptr<Shader> _divergenceCS;
    shared_ptr<Shader> _jacobiCS;
    shared_ptr<Shader> _sourcingCS;
    shared_ptr<Shader> _computeVorticityCS;
    shared_ptr<Shader> _confineVorticityCS;
};