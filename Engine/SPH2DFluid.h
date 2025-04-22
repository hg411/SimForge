#pragma once

#include "Simulation.h"

class Imgui;

class SPH2DFluid : public Simulation {
  public:
    SPH2DFluid();
    virtual ~SPH2DFluid();

    void Update() override;
    void Render() override;

  private:
    void InitShaders() override;
    void InitSimulationObjects() override;

    void BuildUI() override;

  private:
    // Bounding Box
    Vec3 _boxCenter = Vec3(0.0f, 0.0f, 1.0f);
    float _boxWidth = 3.4f;
    float _boxHeight = 1.7f;
};
