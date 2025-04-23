#pragma once
class SwapChain {
  public:
    void Init(const WindowInfo &windowInfo, ComPtr<IDXGIFactory> dxgi,
              ComPtr<ID3D12CommandQueue> cmdQueue);
    void Present();
    void SwapIndex();

    void Resize(const WindowInfo &windowInfo);

    ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
    uint8 GetBackBufferIndex() { return _backBufferIndex; }

  private:
    void CreateSwapChain(const WindowInfo &windowInfo, ComPtr<IDXGIFactory> dxgi,
                         ComPtr<ID3D12CommandQueue> cmdQueue);

  private:
    ComPtr<IDXGISwapChain> _swapChain;
    uint32 _backBufferIndex = 0;
};
