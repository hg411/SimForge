#pragma once
#pragma once

class RootSignature {
  public:
    void Init(ComPtr<ID3D12Device> device);

    ComPtr<ID3D12RootSignature> GetGraphicsRootSignature() { return _graphicsRootSignature; }
    ComPtr<ID3D12RootSignature> GetComputeRootSignature() { return _computeRootSignature; }

  private:
    void CreateGraphicsRootSignature(ComPtr<ID3D12Device> device);
    void CreateComputeRootSignature(ComPtr<ID3D12Device> device);

  private:
    ComPtr<ID3D12RootSignature> _graphicsRootSignature;
    ComPtr<ID3D12RootSignature> _computeRootSignature;
};