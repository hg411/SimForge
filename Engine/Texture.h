#pragma once

#include "Object.h"

class Texture : public Object {
  public:
    Texture();
    virtual ~Texture();

  public:
    void Create(DXGI_FORMAT format, uint32 width, uint32 height, const D3D12_HEAP_PROPERTIES &heapProperty,
                D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_FLAGS resFlags, Vec4 clearColor = Vec4(0.0f));

    void CreateFromResource(ComPtr<ID3D12Resource> tex2D);

    void BindSRVToGraphics(SRV_REGISTER reg, bool forPixelShader);
    void SetGraphicsRootSRV(SRV_REGISTER reg, bool forPixelShader);

    void BindSRVToCompute(SRV_REGISTER reg);
    void BindUAVToCompute(UAV_REGISTER reg);
    void CopyResource(shared_ptr<Texture>& src);

    void Transition(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_RESOURCE_STATES resourceState);

  public:
    ComPtr<ID3D12Resource> GetTex2D() { return _tex2D; }
    ComPtr<ID3D12DescriptorHeap> GetSRV() { return _srvHeap; }
    ComPtr<ID3D12DescriptorHeap> GetRTV() { return _rtvHeap; }
    ComPtr<ID3D12DescriptorHeap> GetDSV() { return _dsvHeap; }
    ComPtr<ID3D12DescriptorHeap> GetUAV() { return _uavHeap; }

    D3D12_RESOURCE_STATES GetResourceState() { return _resourceState; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVHandle() { return _srvHeapBegin; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetUAVHandle() { return _uavHeapBegin; }

    float GetWidth() { return static_cast<float>(_desc.Width); }
    float GetHeight() { return static_cast<float>(_desc.Height); }

  private:
    ScratchImage _image;
    D3D12_RESOURCE_DESC _desc = {};
    ComPtr<ID3D12Resource> _tex2D;

    ComPtr<ID3D12DescriptorHeap> _srvHeap;
    ComPtr<ID3D12DescriptorHeap> _rtvHeap;
    ComPtr<ID3D12DescriptorHeap> _dsvHeap;
    ComPtr<ID3D12DescriptorHeap> _uavHeap;

    D3D12_RESOURCE_STATES _resourceState = {};

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE _srvHeapBegin = {};
    D3D12_CPU_DESCRIPTOR_HANDLE _uavHeapBegin = {};
};
