#include "pch.h"
#include "StructuredBuffer.h"
#include "Engine.h"
#include "Device.h"
#include "TableDescriptorHeap.h"
#include "CommandQueue.h"

StructuredBuffer::StructuredBuffer() {}

StructuredBuffer::~StructuredBuffer() {}

void StructuredBuffer::Init(uint32 elementSize, uint32 elementCount, void *initialData) {
    _elementSize = elementSize;
    _elementCount = elementCount;
    _resourceState = D3D12_RESOURCE_STATE_COMMON;

    // Buffer
    {
        uint64 bufferSize = static_cast<uint64>(_elementSize) * _elementCount;
        D3D12_RESOURCE_DESC desc =
            CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        DEVICE->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, _resourceState, nullptr,
                                        IID_PPV_ARGS(&_buffer));

        if (initialData) // initialData가 있다면 데이터 미리 전달.
            CopyInitialData(bufferSize, initialData);
    }

    // SRV
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DEVICE->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_srvHeap));

        _srvHeapBegin = _srvHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = _elementCount;
        srvDesc.Buffer.StructureByteStride = _elementSize;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        DEVICE->CreateShaderResourceView(_buffer.Get(), &srvDesc, _srvHeapBegin);
    }

    // UAV
    {
        D3D12_DESCRIPTOR_HEAP_DESC uavheapDesc = {};
        uavheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        uavheapDesc.NumDescriptors = 1;
        uavheapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DEVICE->CreateDescriptorHeap(&uavheapDesc, IID_PPV_ARGS(&_uavHeap));

        _uavHeapBegin = _uavHeap->GetCPUDescriptorHandleForHeapStart();

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = _elementCount;
        uavDesc.Buffer.StructureByteStride = _elementSize;
        uavDesc.Buffer.CounterOffsetInBytes = 0;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        DEVICE->CreateUnorderedAccessView(_buffer.Get(), nullptr, &uavDesc, _uavHeapBegin);
    }
}

void StructuredBuffer::SetGraphicsRootSRV(SRV_REGISTER reg, bool forPixelShader) {
    // D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE 사용 방식
    // if (_resourceState != D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE) {
    //     D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState,
    //                                                                           D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    //     RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
    //     _resourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
    // }

    D3D12_RESOURCE_STATES desiredState =
        forPixelShader ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

    if (_resourceState != desiredState) {
        D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState, desiredState);
        GRAPHICS_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = desiredState;
    }

    GRAPHICS_CMD_LIST->SetGraphicsRootShaderResourceView(static_cast<UINT>(reg), _buffer->GetGPUVirtualAddress());
}

void StructuredBuffer::SetGraphicsRootUAV(UAV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        GRAPHICS_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    GRAPHICS_CMD_LIST->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(reg), _buffer->GetGPUVirtualAddress());
}

void StructuredBuffer::SetComputeRootSRV(SRV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }

    COMPUTE_CMD_LIST->SetComputeRootShaderResourceView(static_cast<UINT>(reg), _buffer->GetGPUVirtualAddress());
}

void StructuredBuffer::SetComputeRootUAV(UAV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    COMPUTE_CMD_LIST->SetComputeRootUnorderedAccessView(static_cast<UINT>(reg), _buffer->GetGPUVirtualAddress());
}

void StructuredBuffer::BindSRVToGraphics(SRV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    GEngine->GetGraphicsDescHeap()->SetSRV(_srvHeapBegin, reg);
}

void StructuredBuffer::BindSRVToCompute(SRV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        COMPUTE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    }

    GEngine->GetComputeDescHeap()->SetSRV(_srvHeapBegin, reg);
}

void StructuredBuffer::BindUAVToCompute(UAV_REGISTER reg) {
    if (_resourceState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
        D3D12_RESOURCE_BARRIER barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(_buffer.Get(), _resourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        COMPUTE_CMD_LIST->ResourceBarrier(1, &barrier);
        _resourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    GEngine->GetComputeDescHeap()->SetUAV(_uavHeapBegin, reg);
}

void StructuredBuffer::CopyInitialData(uint64 bufferSize, void *initialData) {
    ComPtr<ID3D12Resource> readBuffer = nullptr;
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE);
    D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    DEVICE->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, IID_PPV_ARGS(&readBuffer));

    uint8 *dataBegin = nullptr;
    D3D12_RANGE readRange{0, 0};
    readBuffer->Map(0, &readRange, reinterpret_cast<void **>(&dataBegin));
    memcpy(dataBegin, initialData, bufferSize);
    readBuffer->Unmap(0, nullptr);

    // Common -> Copy
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

        RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
    }

    RESOURCE_CMD_LIST->CopyBufferRegion(_buffer.Get(), 0, readBuffer.Get(), 0, bufferSize);

    // Copy -> Common
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            _buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
        RESOURCE_CMD_LIST->ResourceBarrier(1, &barrier);
    }

    GEngine->GetGraphicsCmdQueue()->FlushResourceCommandQueue();

    _resourceState = D3D12_RESOURCE_STATE_COMMON;
}