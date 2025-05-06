#include "pch.h"
#include "Device.h"

void Device::Init() {
    // D3D12 Layer
#ifdef _DEBUG
    ::D3D12GetDebugInterface(IID_PPV_ARGS(&_debugController));
    _debugController->EnableDebugLayer();

    //ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dredSettings;
    //if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dredSettings)))) {
    //    dredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    //    dredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
    //}

    //ComPtr<ID3D12Debug1> debug1;
    //if (SUCCEEDED(_debugController.As(&debug1))) {
    //    debug1->SetEnableGPUBasedValidation(TRUE);
    //}
#endif

    // DXGI(DirectX Graphics Infrastructure)
    ::CreateDXGIFactory(IID_PPV_ARGS(&_dxgi));

    // CreateDevice
    ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_device));
}
