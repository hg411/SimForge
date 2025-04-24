#pragma once

class ConstantBuffer {
  public:
    ConstantBuffer();
    ~ConstantBuffer();

    void Init(uint32 size, uint32 count);

    void Clear();
    void SetGraphicsGlobalData(void *buffer, uint32 size);

    void UpdateData(void *buffer, uint32 size);
    void SetGraphicsRootCBV(CBV_REGISTER reg);
    void SetComputeRootCBV(CBV_REGISTER reg);

    //void SetData(void *buffer, uint32 size);
    void BindToGraphics(CBV_REGISTER reg);
    void BindToCompute(CBV_REGISTER reg);
    void PushGraphicsData(void *buffer, uint32 size, CBV_REGISTER reg);
    void PushComputeData(void *buffer, uint32 size, CBV_REGISTER reg);

    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(uint32 index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(uint32 index);

  private:
    void CreateBuffer();
    void CreateView();

  private:
    ComPtr<ID3D12Resource> _cbvBuffer;
    BYTE *_mappedBuffer = nullptr;
    uint32 _elementSize = 0;
    uint32 _elementCount = 0;

    ComPtr<ID3D12DescriptorHeap> _cbvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandleBegin = {};
    uint32 _handleIncrementSize = 0;

    uint32 _currentIndex = 0;
};
