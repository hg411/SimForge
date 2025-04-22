#include "pch.h"
#include "SimulationManager.h"
#include "Simulation.h"
#include "SPH2DFluid.h"

void SimulationManager::Update() {
    if (_activeSimulation == nullptr)
        return;

    _activeSimulation->Update();
    _activeSimulation->LateUpdate();
    _activeSimulation->FinalUpdate();
}

void SimulationManager::Render() {
    if (_activeSimulation) {
        _activeSimulation->Render();
    }
}

void SimulationManager::LoadSimulation() {
    //_activeSimulation = LoadTestScene();
    _activeSimulation = LoadSPH2DFluid();

    _activeSimulation->Awake();
    _activeSimulation->Start();
}

shared_ptr<Simulation> SimulationManager::LoadSPH2DFluid() { 
    shared_ptr<Simulation> sph2DFluid = make_shared<SPH2DFluid>();
    sph2DFluid->Init();

    return sph2DFluid;
}
