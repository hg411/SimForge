#pragma once

#include "Simulation.h"

class StableFluid : public Simulation {
  public:
    StableFluid();
    virtual ~StableFluid();

    void Init() override;

    void Update() override;
    void FinalUpdate() override;
    void Render() override;

  private:
    void InitShaders() override;
    void InitConstantBuffers() override;
    void InitStructuredBuffers() override;
    void InitSimulationObjects() override;

    void BuildUI() override;

  private:
};
