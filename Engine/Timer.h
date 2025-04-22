#pragma once

class Timer {
    DECLARE_SINGLE(Timer)

  public:
    void Init();
    void Update();

    void Start(bool measureGPU);
    void End();

    uint32 GetFps() { return _fps; }
    float GetDeltaTime() { return _deltaTime; }

  private:
    uint64 _frequency = 0;
    uint64 _prevCount = 0;
    float _deltaTime = 0.f;

    uint32 _frameCount = 0;
    float _frameTime = 0.f;
    uint32 _fps = 0;

    ComPtr<ID3D12QueryHeap> _queryHeap;
    ComPtr<ID3D12Resource> _queryResultBuffer;

    UINT64 _startTimestamp = 0;
    UINT64 _endTimestamp = 0;
    UINT64 _timestampFrequency = 0;
    double _elapsedTimeCPU = 0.0;
    double _elapsedTimeGPU = 0.0;
    decltype(std::chrono::high_resolution_clock::now()) _startTimeCPU;

    bool _measureGPU = false;
};
