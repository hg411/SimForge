#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine.h"
#include "Device.h"
#include "TableDescriptorHeap.h"
#include "CommandQueue.h"

ConstantBuffer::ConstantBuffer() {}

ConstantBuffer::~ConstantBuffer() {
    if (_cbvBuffer) {
        if (_cbvBuffer != nullptr)
            _cbvBuffer->Unmap(0, nullptr);

        _cbvBuffer = nullptr;
    }
}

void ConstantBuffer::Init(uint32 size, uint32 count) {
    // 상수 버퍼는 256 바이트 배수로 만들어야 함.
    // 0 256 512 768
    _elementSize = (size + 255) & ~255;
    _elementCount = count;

    CreateBuffer();
    CreateView();
}

void ConstantBuffer::Clear() { _currentIndex = 0; }

void ConstantBuffer::SetGraphicsGlobalData(void *buffer, uint32 size) {
    assert(_elementSize == ((size + 255) & ~255));
    ::memcpy(&_mappedBuffer[0], buffer, size);
    GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(0, _cbvBuffer->GetGPUVirtualAddress());
}

void ConstantBuffer::UpdateData(void *buffer, uint32 size) {
    assert(_elementSize == ((size + 255) & ~255));

    ::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);
}

void ConstantBuffer::SetGraphicsRootCBV(CBV_REGISTER reg) {
    GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(static_cast<UINT>(reg), _cbvBuffer->GetGPUVirtualAddress());
}

void ConstantBuffer::SetComputeRootCBV(CBV_REGISTER reg) {
    COMPUTE_CMD_LIST->SetComputeRootConstantBufferView(static_cast<UINT>(reg), _cbvBuffer->GetGPUVirtualAddress());
}

//void ConstantBuffer::SetData(void *buffer, uint32 size) { 
//    assert(_currentIndex < _elementCount);
//    assert(_elementSize == ((size + 255) & ~255));
//    ::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);
//
//    _currentIndex++;
//}

void ConstantBuffer::BindToGraphics(CBV_REGISTER reg) {
    assert(_currentIndex < _elementCount);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = GetCpuHandle(_currentIndex);
    GEngine->GetGraphicsDescHeap()->SetCBV(handle, reg);

    _currentIndex++;
}

void ConstantBuffer::BindToCompute(CBV_REGISTER reg) {
    assert(_currentIndex < _elementCount);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
    GEngine->GetComputeDescHeap()->SetCBV(cpuHandle, reg);

    //_currentIndex++;
}

void ConstantBuffer::PushGraphicsData(void *buffer, uint32 size, CBV_REGISTER reg) {
    assert(_currentIndex < _elementCount);
    assert(_elementSize == ((size + 255) & ~255));

    ::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);

    GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, reg);
    
    _currentIndex++;
}

void ConstantBuffer::PushComputeData(void *buffer, uint32 size, CBV_REGISTER reg) {
    UpdateData(buffer, size);
    BindToCompute(reg);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuVirtualAddress(uint32 index) {
    D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = _cbvBuffer->GetGPUVirtualAddress();
    objCBAddress += index * _elementSize;
    return objCBAddress;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index) {
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHandleBegin, index * _handleIncrementSize);
}

void ConstantBuffer::CreateBuffer() {
    uint32 bufferSize = _elementSize * _elementCount;
    D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    DEVICE->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                    nullptr, IID_PPV_ARGS(&_cbvBuffer));

    _cbvBuffer->Map(0, nullptr, reinterpret_cast<void **>(&_mappedBuffer));
    // We do not need to unmap until we are done with the resource.  However, we must not write to
    // the resource while it is in use by the GPU (so we must use synchronization techniques).
}

void ConstantBuffer::CreateView() {
    D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
    cbvDesc.NumDescriptors = _elementCount;
    cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&_cbvHeap));

    _cpuHandleBegin = _cbvHeap->GetCPUDescriptorHandleForHeapStart();
    _handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (uint32 i = 0; i < _elementCount; ++i) {
        D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = _cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(_elementSize) * i;
        cbvDesc.SizeInBytes = _elementSize; // CB size is required to be 256-byte aligned.

        DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
    }
}
