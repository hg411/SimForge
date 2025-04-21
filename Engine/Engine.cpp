#include "pch.h"
#include "Engine.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "RootSignature.h"
#include "TableDescriptorHeap.h"
#include "RenderTargetGroup.h"
#include "Texture.h"

void Engine::Init(const WindowInfo &windowInfo) {
    _windowInfo = windowInfo;

    // 그려질 화면 크기를 설정
    _viewport = {0, 0, static_cast<FLOAT>(windowInfo.width), static_cast<FLOAT>(windowInfo.height), 0.0f, 1.0f};
    _scissorRect = CD3DX12_RECT(0, 0, windowInfo.width, windowInfo.height);

    // Device
    _device = make_shared<Device>();
    _device->Init();

    // Command Queue
    _graphicsCmdQueue = make_shared<GraphicsCommandQueue>();
    _graphicsCmdQueue->Init(_device->GetDevice());

    _computeCmdQueue = make_shared<ComputeCommandQueue>();
    _computeCmdQueue->Init(_device->GetDevice());

    // SwapChain
    _swapChain = make_shared<SwapChain>();
    _swapChain->Init(windowInfo, _device->GetDevice(), _device->GetDXGI(), _graphicsCmdQueue->GetCmdQueue());

    _graphicsCmdQueue->SetSwapChain(_swapChain);

    // RootSignature
    _rootSignature = make_shared<RootSignature>();
    _rootSignature->Init(_device->GetDevice());

    // Descriptor Heap
    _graphicsDescHeap = make_shared<GraphicsDescriptorHeap>();
    _graphicsDescHeap->Init(_device->GetDevice(), 256);

    _computeDescHeap = make_shared<ComputeDescriptorHeap>();
    _computeDescHeap->Init(_device->GetDevice());

    // CreateConstantBuffer(CBV_REGISTER::b0, sizeof(LightParams), 1);
    // CreateConstantBuffer(CBV_REGISTER::b1, sizeof(TransformParams), 256);
    // CreateConstantBuffer(CBV_REGISTER::b2, sizeof(MaterialParams), 256);

    CreateRenderTargetGroups();

    ResizeWindow(windowInfo.width, windowInfo.height);

    // GET_SINGLE(Input)->Init(info.hwnd);
    // GET_SINGLE(Timer)->Init();
    // GET_SINGLE(Resources)->Init();
}

void Engine::Update() {}

void Engine::ResizeWindow(int32 width, int32 height) {
    _windowInfo.width = width;
    _windowInfo.height = height;

    RECT rect = {0, 0, width, height};
    ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    //::SetWindowPos(_windowInfo.hwnd, 0, 100, 100, width, height, 0);
    ::SetWindowPos(_windowInfo.hwnd, 0, 100, 100, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Engine::CreateRenderTargetGroups() {
    // DepthStencil
    shared_ptr<Texture> dsTexture = make_shared<Texture>();
    dsTexture->Create(DXGI_FORMAT_D32_FLOAT, _windowInfo.width, _windowInfo.height,
                      CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                      D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    // SwapChain Group
    {
        vector<RenderTarget> rtVec(SWAP_CHAIN_BUFFER_COUNT);

        for (uint32 i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i) {
            ComPtr<ID3D12Resource> resource;
            _swapChain->GetSwapChain()->GetBuffer(i, IID_PPV_ARGS(&resource));

            rtVec[i].target = make_shared<Texture>();
            rtVec[i].target->CreateFromResource(resource);
        }

        _rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)] = make_shared<RenderTargetGroup>();
        _rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)]->Create(
            RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN, rtVec, dsTexture);
    }
}
