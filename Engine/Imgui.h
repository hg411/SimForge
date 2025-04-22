#pragma once

class Simulation;

class Imgui {
  public:
    Imgui();
    ~Imgui();

    void Init();
    void Update(shared_ptr<Simulation> simulation);
    void Render();

    private:
    ComPtr<ID3D12DescriptorHeap> _srvHeap;
    int _frameCount = 0;
};
