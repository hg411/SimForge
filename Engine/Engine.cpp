#include "pch.h"
#include "Engine.h"
#include "Device.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "RootSignature.h"
#include "TableDescriptorHeap.h"
#include "RenderTargetGroup.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "CBufferStructs.h"
#include "Input.h"
#include "Timer.h"
#include "SimulationManager.h"
#include "Resources.h"

void Engine::Init(const WindowInfo &windowInfo) {
    _windowInfo = windowInfo;

    _viewport = {0, 0, static_cast<FLOAT>(windowInfo.width), static_cast<FLOAT>(windowInfo.height), 0.0f, 1.0f};
    _scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(windowInfo.width), static_cast<LONG>(windowInfo.height));

    AdjustWindowSizeAndPosition();

    // Device
    _device = make_shared<Device>();
    _device->Init();

    // Command Queue
    _graphicsCmdQueue = make_shared<GraphicsCommandQueue>();
    _graphicsCmdQueue->Init(_device->GetDevice());

    _computeCmdQueue = make_shared<ComputeCommandQueue>();
    _computeCmdQueue->Init(_device->GetDevice());

    // SwapChain
    InitSwapChain();

    _graphicsCmdQueue->SetSwapChain(_swapChain);

    // RootSignature
    _rootSignature = make_shared<RootSignature>();
    _rootSignature->Init(_device->GetDevice());

    // Descriptor Heap
    // �ϴ� Root Descriptor�� ����ϴ� ������� ���.
    //_graphicsDescHeap = make_shared<GraphicsDescriptorHeap>();
    //_graphicsDescHeap->Init(256);

    //_computeDescHeap = make_shared<ComputeDescriptorHeap>();
    //_computeDescHeap->Init(256);

    _globalParamsCB = make_shared<ConstantBuffer>();
    _globalParamsCB->Init(sizeof(GlobalParams), 1);

    _transformParamsCB = make_shared<ConstantBuffer>();
    _transformParamsCB->Init(sizeof(TransformParams), 1);
    //_transformParamsCB->Init(sizeof(TransformParams), 256);

    _materialParamsCB = make_shared<ConstantBuffer>();
    _materialParamsCB->Init(sizeof(MaterialParams), 1);
    //_materialParamsCB->Init(sizeof(MaterialParams), 256);

    CreateRenderTargetGroups();

    GET_SINGLE(Input)->Init(windowInfo.hwnd);
    GET_SINGLE(Timer)->Init();
}

void Engine::Update() {
    CheckResizeByClientRect();
    GET_SINGLE(Input)->Update();
    GET_SINGLE(Timer)->Update();
    GET_SINGLE(SimulationManager)->Update();

    Render();
}

void Engine::Render() {
    RenderBegin();

    GET_SINGLE(SimulationManager)->Render();

    RenderEnd();
}

void Engine::RenderBegin() { _graphicsCmdQueue->RenderBegin(); }

void Engine::RenderEnd() { _graphicsCmdQueue->RenderEnd(); }

void Engine::ResizeWindow(const WindowInfo &windowInfo) {
    if (windowInfo.width <= 0 || windowInfo.height <= 0)
        return;

    if (_swapChain) {
        _viewport = {0, 0, static_cast<FLOAT>(_windowInfo.width), static_cast<FLOAT>(_windowInfo.height), 0.0f, 1.0f};
        _scissorRect = CD3DX12_RECT(0, 0, _windowInfo.width, _windowInfo.height);

        GRAPHICS_CMD_LIST->Reset(_graphicsCmdQueue->GetCmdAlloc().Get(), nullptr);

        GRAPHICS_CMD_LIST->RSSetViewports(1, &_viewport);
        GRAPHICS_CMD_LIST->RSSetScissorRects(1, &_scissorRect);

        GRAPHICS_CMD_LIST->Close();

        GetGraphicsCmdQueue()->WaitSync();

        _rtGroups[static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::SWAP_CHAIN)].reset();
        _swapChain->Resize(_windowInfo);

        CreateRenderTargetGroups();
    }
}

void Engine::CheckResizeByClientRect() {
    RECT clientRect;
    ::GetClientRect(_windowInfo.hwnd, &clientRect);
    int32 width = clientRect.right - clientRect.left;
    int32 height = clientRect.bottom - clientRect.top;

    // === [��üȭ�� ���� �Ǵ�] ===
    RECT winRect;
    GetWindowRect(_windowInfo.hwnd, &winRect);

    HMONITOR hMonitor = MonitorFromWindow(_windowInfo.hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {sizeof(MONITORINFO)};
    GetMonitorInfo(hMonitor, &mi);

    LONG style = GetWindowLong(_windowInfo.hwnd, GWL_STYLE);
    bool isBorderless = (style & WS_OVERLAPPEDWINDOW) == 0;
    bool isFullscreen = isBorderless && EqualRect(&winRect, &mi.rcMonitor);

    // ��üȭ���̸� ����� �ػ󵵷� ���� ����
    if (isFullscreen) {
        width = mi.rcMonitor.right - mi.rcMonitor.left;
        height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    // �� windowInfo ����
    WindowInfo newInfo = _windowInfo;
    newInfo.width = width;
    newInfo.height = height;
    newInfo.windowed = !isFullscreen;

    if (_windowInfo.width != width || _windowInfo.height != height || _windowInfo.windowed != newInfo.windowed) {
        _windowInfo = newInfo;
        ResizeWindow(_windowInfo);
    }
}

void Engine::AdjustWindowSizeAndPosition() {
    RECT rect = {0, 0, _windowInfo.width, _windowInfo.height};
    ::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    ::SetWindowPos(_windowInfo.hwnd, 0, 100, 100, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Engine::InitSwapChain() {
    _swapChain = make_shared<SwapChain>();
    _swapChain->Init(_windowInfo, _device->GetDXGI(), _graphicsCmdQueue->GetCmdQueue());
}

shared_ptr<RenderTargetGroup> Engine::GetRTGroup(RENDER_TARGET_GROUP_TYPE type) {
    return _rtGroups[static_cast<uint8>(type)];
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
