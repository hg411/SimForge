#include "pch.h"
#include "Texture.h"
#include "Engine.h"
#include "Device.h"
#include "CommandQueue.h"
#include "TableDescriptorHeap.h" 

Texture::Texture() : Object(OBJECT_TYPE::TEXTURE) {}

Texture::~Texture() {}

void Texture::Create(DXGI_FORMAT format, uint32 width, uint32 height, const D3D12_HEAP_PROPERTIES &heapProperty,
                     D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vec4 clearColor) {
    _desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
    _desc.Flags = resFlags;

    D3D12_CLEAR_VALUE optimizedClearValue = {};
    D3D12_CLEAR_VALUE *pOptimizedClearValue = nullptr;

   _resourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;

    if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
        _resourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE;
        optimizedClearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);
        pOptimizedClearValue = &optimizedClearValue;
    } else if (resFlags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
        _resourceState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
        float arrFloat[4] = {clearColor.x, clearColor.y, clearColor.z, clearColor.w};
        optimizedClearValue = CD3DX12_CLEAR_VALUE(format, arrFloat);
        pOptimizedClearValue = &optimizedClearValue;
    }

    // Create Texture2D
    HRESULT hr = DEVICE->CreateCommittedResource(&heapProperty, heapFlags, &_desc, _resourceState, pOptimizedClearValue,
                                                 IID_PPV_ARGS(&_tex2D));

    assert(SUCCEEDED(hr));

    CreateFromResource(_tex2D);
}

void Texture::CreateFromResource(ComPtr<ID3D12Resource> tex2D) {
    _tex2D = tex2D;

    _desc = tex2D->GetDesc();

    // 주요 조합
    // - DSV 단독 (조합X)
    // - SRV
    // - RTV + SRV
    if (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
        // DSV(Depth Stencil View)
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        heapDesc.NumDescriptors = 1;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;
        DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_dsvHeap));

        D3D12_CPU_DESCRIPTOR_HANDLE hDSVHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
        DEVICE->CreateDepthStencilView(_tex2D.Get(), nullptr, hDSVHandle);
    } else {
        if (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
            // RTV(Render Target View)
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            heapDesc.NumDescriptors = 1;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask = 0;
            DEVICE->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeap));

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapBegin = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
            DEVICE->CreateRenderTargetView(_tex2D.Get(), nullptr, rtvHeapBegin);
        }

        if (_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
            // UAV
            D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
            uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            uavHeapDesc.NumDescriptors = 1;
            uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            uavHeapDesc.NodeMask = 0;
            DEVICE->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&_uavHeap));

            _uavHeapBegin = _uavHeap->GetCPUDescriptorHandleForHeapStart();

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = _image.GetMetadata().format;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

            DEVICE->CreateUnorderedAccessView(_tex2D.Get(), nullptr, &uavDesc, _uavHeapBegin);
        }

        // SRV(Shader Resource View)
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_srvHeap));

        _srvHeapBegin = _srvHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = _image.GetMetadata().format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        DEVICE->CreateShaderResourceView(_tex2D.Get(), &srvDesc, _srvHeapBegin);
    }
}

void Texture::BindSRVToCompute(SRV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _tex2D.Get(), _resourceState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        COMPUTE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }

    GEngine->GetComputeDescHeap()->SetSRV(_srvHeapBegin, reg);
}

void Texture::BindUAVToCompute(UAV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_tex2D.Get(), _resourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        COMPUTE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    GEngine->GetComputeDescHeap()->SetUAV(_uavHeapBegin, reg);
}
