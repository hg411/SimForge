#pragma once

class SimulationObject;
class Camera;
class Imgui;

class Simulation : public enable_shared_from_this<Simulation> {
  public:
    Simulation();
    virtual ~Simulation();

    void Init();

    virtual void Awake();
    virtual void Start();
    virtual void Update();
    virtual void LateUpdate();
    virtual void FinalUpdate();

    shared_ptr<Camera> GetMainCamera();

    virtual void Render();

    void PushGlobalData();
    void ClearRTV();

  public:
    void AddSimulationObject(shared_ptr<SimulationObject> simulationObject);
    void AddCamera(shared_ptr<Camera> camera);
    virtual void BuildUI() {}

  protected:
    void InitImgui();
    virtual void InitShaders() {}
    virtual void InitSimulationObjects() {}

  protected:
    vector<shared_ptr<SimulationObject>> _simulationObjects;
    vector<shared_ptr<Camera>> _cameras;
    shared_ptr<Imgui> _imgui;
};
