#pragma once

class Device;
class SwapChain;
class GraphicsCommandQueue;
class ComputeCommandQueue;
class RootSignature;
class GraphicsDescriptorHeap;
class ComputeDescriptorHeap;
class RenderTargetGroup;
class ConstantBuffer;

enum class RENDER_TARGET_GROUP_TYPE : uint8;

class Engine {
  public:
    void Init(const WindowInfo &windowInfo);
    void Update();

    void Render();
    void RenderBegin();
    void RenderEnd();

    void ResizeWindow(int32 width, int32 height);
    void CheckResizeByClientRect();

    void AdjustWindowSizeAndPosition(int32 width, int32 height);

  private:
    void InitSwapChain();

  public:
    const WindowInfo &GetWindowInfo() { return _windowInfo; }
    shared_ptr<Device> GetDevice() { return _device; }
    shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
    shared_ptr<ComputeCommandQueue> GetComputeCmdQueue() { return _computeCmdQueue; }
    shared_ptr<SwapChain> GetSwapChain() { return _swapChain; }
    shared_ptr<RootSignature> GetRootSignature() { return _rootSignature; }
    shared_ptr<GraphicsDescriptorHeap> GetGraphicsDescHeap() { return _graphicsDescHeap; }
    shared_ptr<ComputeDescriptorHeap> GetComputeDescHeap() { return _computeDescHeap; }
    shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type);
    shared_ptr<ConstantBuffer> GetGlobalParamsCB() { return _globalParamsCB; }
    shared_ptr<ConstantBuffer> GetTransformParamsCB() { return _transformParamsCB; }
    shared_ptr<ConstantBuffer> GetMaterialParamsCB() { return _materialParamsCB; }

  private:
    void CreateRenderTargetGroups();

  private:
    // 그려질 화면 크기 관련
    WindowInfo _windowInfo;
    D3D12_VIEWPORT _viewport = {};
    D3D12_RECT _scissorRect = {};

    shared_ptr<Device> _device;
    shared_ptr<GraphicsCommandQueue> _graphicsCmdQueue;
    shared_ptr<ComputeCommandQueue> _computeCmdQueue;
    shared_ptr<SwapChain> _swapChain;
    shared_ptr<RootSignature> _rootSignature;
    shared_ptr<GraphicsDescriptorHeap> _graphicsDescHeap;
    shared_ptr<ComputeDescriptorHeap> _computeDescHeap;
    array<shared_ptr<RenderTargetGroup>, 1> _rtGroups;
    shared_ptr<ConstantBuffer> _globalParamsCB;
    shared_ptr<ConstantBuffer> _transformParamsCB;
    shared_ptr<ConstantBuffer> _materialParamsCB;
    ////vector<shared_ptr<ConstantBuffer>> _constantBuffers;
};
