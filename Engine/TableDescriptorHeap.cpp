#include "pch.h"
#include "TableDescriptorHeap.h"
#include "Engine.h"
#include "CommandQueue.h"

// ************************
// GraphicsDescriptorHeap
// ************************

void GraphicsDescriptorHeap::Init(ComPtr<ID3D12Device> device, uint32 count) {
    _device = device;
    _groupCount = count;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = count * (CBV_SRV_REGISTER_COUNT - 1); // b0는 전역
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap));

    _handleSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    _groupSize = _handleSize * (CBV_SRV_REGISTER_COUNT - 1); // b0는 전역
}

void GraphicsDescriptorHeap::Clear() { _currentGroupIndex = 0; }

void GraphicsDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

    uint32 destRange = 1;
    uint32 srcRange = 1;
    _device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange,
                             D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

    uint32 destRange = 1;
    uint32 srcRange = 1;
    _device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange,
                             D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GraphicsDescriptorHeap::CommitTable() {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap->GetGPUDescriptorHandleForHeapStart();

    // param[1] = CBV Table
    handle.ptr += _currentGroupIndex * _groupSize;
    GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(static_cast<uint32>(ROOT_PARAM_GRAPHICS::CBV_TABLE), handle);

    // param[2] = SRV Table
    handle.ptr += _handleSize * (CBV_REGISTER_COUNT - 1);
    GRAPHICS_CMD_LIST->SetGraphicsRootDescriptorTable(static_cast<uint32>(ROOT_PARAM_GRAPHICS::SRV_TABLE), handle);

    _currentGroupIndex++;
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(CBV_REGISTER reg) {
    return GetCPUHandle(static_cast<uint32>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(SRV_REGISTER reg) {
    return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE GraphicsDescriptorHeap::GetCPUHandle(uint8 reg) {
    assert(reg > 0 && "b0 is a global root CBV and not managed by DescriptorHeap");
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += _currentGroupIndex * _groupSize;
    handle.ptr += (reg - 1) * _handleSize;
    return handle;
}

// ************************
// ComputeDescriptorHeap
// ************************

void ComputeDescriptorHeap::Init(ComPtr<ID3D12Device> device) {
    _device = device;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = TOTAL_REGISTER_COUNT;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    _device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap));

    _handleSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetCBV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, CBV_REGISTER reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

    uint32 destRange = 1;
    uint32 srcRange = 1;
    _device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange,
                             D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetSRV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, SRV_REGISTER reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

    uint32 destRange = 1;
    uint32 srcRange = 1;
    _device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange,
                             D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::SetUAV(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, UAV_REGISTER reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE destHandle = GetCPUHandle(reg);

    uint32 destRange = 1;
    uint32 srcRange = 1;
    _device->CopyDescriptors(1, &destHandle, &destRange, 1, &srcHandle, &srcRange,
                             D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ComputeDescriptorHeap::CommitTable() {
    // compute shader를 쓰기전에(Update 시작시점에서 실행시켜 줘야함) 나중에 수정필요!!!
    // ID3D12DescriptorHeap *descHeap = _descHeap.Get();
    // cmdList->SetDescriptorHeaps(1, &descHeap);

    D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap.Get()->GetGPUDescriptorHandleForHeapStart();

    // param[0] = CBV Table
    COMPUTE_CMD_LIST->SetComputeRootDescriptorTable(static_cast<uint32>(ROOT_PARAM_COMPUTE::CBV_TABLE), handle);

    // param[1] = SRV Table
    handle.ptr += _handleSize * CBV_REGISTER_COUNT;
    COMPUTE_CMD_LIST->SetComputeRootDescriptorTable(static_cast<uint32>(ROOT_PARAM_COMPUTE::SRV_TABLE), handle);

    // param[2] = UAV Table
    handle.ptr += _handleSize * SRV_REGISTER_COUNT;
    COMPUTE_CMD_LIST->SetComputeRootDescriptorTable(static_cast<uint32>(ROOT_PARAM_COMPUTE::UAV_TABLE), handle);
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(CBV_REGISTER reg) {
    return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(SRV_REGISTER reg) {
    return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(UAV_REGISTER reg) {
    return GetCPUHandle(static_cast<uint8>(reg));
}

D3D12_CPU_DESCRIPTOR_HANDLE ComputeDescriptorHeap::GetCPUHandle(uint8 reg) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += reg * _handleSize;

    return handle;
}