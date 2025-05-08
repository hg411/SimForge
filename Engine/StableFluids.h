#pragma once

#include "Simulation.h"

class Texture;

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
    uint32 _width;
    uint32 _height;

    shared_ptr<Texture> _velocity;
};
