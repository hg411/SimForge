#include "pch.h"
#include "RootSignature.h"
#include "Engine.h"
#include "CommandQueue.h"

void RootSignature::Init(ComPtr<ID3D12Device> device) {
    CreateGraphicsRootSignature(device);
    CreateComputeRootSignature(device);
}

void RootSignature::CreateGraphicsRootSignature(ComPtr<ID3D12Device> device) {
    D3D12_STATIC_SAMPLER_DESC samplers[] = {
        CD3DX12_STATIC_SAMPLER_DESC(0, // register(s0)
                                    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),

        CD3DX12_STATIC_SAMPLER_DESC(1, // register(s1)
                                    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),

        CD3DX12_STATIC_SAMPLER_DESC(2, // register(s2)
                                    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),

        CD3DX12_STATIC_SAMPLER_DESC(3, // register(s3)
                                    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),

        CD3DX12_STATIC_SAMPLER_DESC(4, // register(s4)
                                    D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),

        CD3DX12_STATIC_SAMPLER_DESC(5, // register(s5)
                                    D3D12_FILTER_MAXIMUM_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),
    };

    //CD3DX12_ROOT_PARAMETER param[TOTAL_REGISTER_COUNT];

    //// CBV
    //for (UINT i = 0; i < static_cast<UINT>(CBV_REGISTER_COUNT); ++i) {
    //    param[i].InitAsConstantBufferView(i);
    //}

    //// SRV
    //for (UINT i = 0; i < static_cast<UINT>(SRV_REGISTER_COUNT); ++i) {
    //    UINT idx = static_cast<UINT>(SRV_REGISTER::t0) + i;
    //    param[idx].InitAsShaderResourceView(i);
    //}

    //// UAV
    //for (UINT i = 0; i < static_cast<UINT>(UAV_REGISTER_COUNT); ++i) {
    //    UINT idx = static_cast<UINT>(UAV_REGISTER::u0) + i;
    //    param[idx].InitAsUnorderedAccessView(i);
    //}

    // Descriptor Table 사용
     CD3DX12_DESCRIPTOR_RANGE cbvRange =
        CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT - 1, 1); // b1~b9

     CD3DX12_DESCRIPTOR_RANGE srvRange =
         CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0); // t0~t9

     CD3DX12_ROOT_PARAMETER param[3];
     param[0].InitAsConstantBufferView(0);
     param[1].InitAsDescriptorTable(1, &cbvRange);
     param[2].InitAsDescriptorTable(1, &srvRange);

    D3D12_ROOT_SIGNATURE_DESC sigDesc =
        CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, _countof(samplers), samplers);
    sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // Graphics 용도, 입력 조립기 단계

    ComPtr<ID3DBlob> blobSignature;
    ComPtr<ID3DBlob> blobError;
    ::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blobSignature, &blobError);
    device->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(),
                                IID_PPV_ARGS(&_graphicsRootSignature));
}

void RootSignature::CreateComputeRootSignature(ComPtr<ID3D12Device> device) {
    D3D12_STATIC_SAMPLER_DESC samplers[] = {
        CD3DX12_STATIC_SAMPLER_DESC(0, // register(s0)
                                    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                    D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP),
    };

    //CD3DX12_ROOT_PARAMETER param[TOTAL_REGISTER_COUNT];

    // Root Descriptor 사용 version
    //// CBV
    //for (UINT i = 0; i < static_cast<UINT>(CBV_REGISTER_COUNT); ++i) {
    //    param[i].InitAsConstantBufferView(i);
    //}

    //// SRV
    //for (UINT i = 0; i < static_cast<UINT>(SRV_REGISTER_COUNT); ++i) {
    //    UINT idx = static_cast<UINT>(SRV_REGISTER::t0) + i;
    //    param[idx].InitAsShaderResourceView(i);
    //}

    //// UAV
    //for (UINT i = 0; i < static_cast<UINT>(UAV_REGISTER_COUNT); ++i) {
    //    UINT idx = static_cast<UINT>(UAV_REGISTER::u0) + i;
    //    param[idx].InitAsUnorderedAccessView(i);
    //}

    // Descriptor Table 사용 version
    CD3DX12_DESCRIPTOR_RANGE cbvRange =
        CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, CBV_REGISTER_COUNT, 0); // b0 ~ b9

    CD3DX12_DESCRIPTOR_RANGE srvRange =
        CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, SRV_REGISTER_COUNT, 0); // t0 ~ t9

    CD3DX12_DESCRIPTOR_RANGE uavRange =
        CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UAV_REGISTER_COUNT, 0); // u0 ~ u9


    CD3DX12_ROOT_PARAMETER param[3];
    param[0].InitAsDescriptorTable(1, &cbvRange);
    param[1].InitAsDescriptorTable(1, &srvRange);
    param[2].InitAsDescriptorTable(1, &uavRange);

    D3D12_ROOT_SIGNATURE_DESC sigDesc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, _countof(samplers), samplers);
    sigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute 용도

    ComPtr<ID3DBlob> blobSignature;
    ComPtr<ID3DBlob> blobError;
    ::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1, blobSignature.GetAddressOf(),
                                  blobError.GetAddressOf());
    device->CreateRootSignature(0, blobSignature->GetBufferPointer(), blobSignature->GetBufferSize(),
                                IID_PPV_ARGS(&_computeRootSignature));

    COMPUTE_CMD_LIST->SetComputeRootSignature(_computeRootSignature.Get()); // 처음 만들 때 Set.
}
