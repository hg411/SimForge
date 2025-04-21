#include "pch.h"
#include "Device.h"

void Device::Init() {
    // #ifdef _DEBUG
    //     ComPtr<ID3D12Debug> debugController;
    //     if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    //         debugController->EnableDebugLayer();
    //     } else {
    //         OutputDebugString(L"Failed to enable debug layer.");
    //     }
    // #endif

    // D3D12 Layer
#ifdef _DEBUG
    ::D3D12GetDebugInterface(IID_PPV_ARGS(&_debugController));
    _debugController->EnableDebugLayer();
#endif

    // DXGI(DirectX Graphics Infrastructure)
    ::CreateDXGIFactory(IID_PPV_ARGS(&_dxgi));

    // CreateDevice
    ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
}
