#pragma once

class Simulation;

class SimulationManager {
    DECLARE_SINGLE(SimulationManager)

  public:
    void Update();
    void Render();
    void LoadSimulation();

    shared_ptr<Simulation> LoadSPH2DFluid();
    shared_ptr<Simulation> LoadSPH3DFluid();
    shared_ptr<Simulation> LoadStableFluid();

    shared_ptr<Simulation> GetActiveSimulation() { return _activeSimulation; }

  private:
    shared_ptr<Simulation> _activeSimulation;

};
