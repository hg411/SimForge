#include "pch.h"
#include "Shader.h"
#include "Engine.h"
#include "Device.h"
#include "RootSignature.h"
#include "CommandQueue.h"

Shader::Shader() : Object(OBJECT_TYPE::SHADER) {}

Shader::~Shader() {}

void Shader::CreateGraphicsShader(ShaderInfo shaderInfo) {
    _shaderInfo = shaderInfo;

    _graphicsPipelineDesc.InputLayout = {shaderInfo.inputLayout.data(),
                                          static_cast<UINT>(shaderInfo.inputLayout.size())};
    _graphicsPipelineDesc.pRootSignature = GRAPHICS_ROOT_SIGNATURE.Get();
    _graphicsPipelineDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    _graphicsPipelineDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    _graphicsPipelineDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    _graphicsPipelineDesc.SampleMask = UINT_MAX;
    _graphicsPipelineDesc.PrimitiveTopologyType = GetTopologyType(shaderInfo.topology);
    _graphicsPipelineDesc.NumRenderTargets = 1;
    _graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    _graphicsPipelineDesc.SampleDesc.Count = 1;
    _graphicsPipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    switch (shaderInfo.shaderType) {
    case SHADER_TYPE::FORWARD:
        _graphicsPipelineDesc.NumRenderTargets = 1;
        _graphicsPipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    }

    switch (shaderInfo.rasterizerType) {
    case RASTERIZER_TYPE::CULL_BACK:
        _graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        _graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        break;
    case RASTERIZER_TYPE::CULL_FRONT:
        _graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        _graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
        break;
    case RASTERIZER_TYPE::CULL_NONE:
        _graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        _graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        break;
    case RASTERIZER_TYPE::WIREFRAME:
        _graphicsPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
        _graphicsPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        break;
    }

    switch (shaderInfo.depthStencilType) {
    case DEPTH_STENCIL_TYPE::LESS:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = TRUE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        break;
    case DEPTH_STENCIL_TYPE::LESS_EQUAL:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = TRUE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        break;
    case DEPTH_STENCIL_TYPE::GREATER:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = TRUE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
        break;
    case DEPTH_STENCIL_TYPE::GREATER_EQUAL:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = TRUE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        break;
    case DEPTH_STENCIL_TYPE::NO_DEPTH_TEST:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = FALSE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        break;
    case DEPTH_STENCIL_TYPE::NO_DEPTH_TEST_NO_WRITE:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = FALSE;
        _graphicsPipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        break;
    case DEPTH_STENCIL_TYPE::LESS_NO_WRITE:
        _graphicsPipelineDesc.DepthStencilState.DepthEnable = TRUE;
        _graphicsPipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        _graphicsPipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        break;
    }

    D3D12_RENDER_TARGET_BLEND_DESC &rt = _graphicsPipelineDesc.BlendState.RenderTarget[0];

    // SrcBlend = Pixel Shader
    // DestBlend = Render Target
    switch (shaderInfo.blendType) {
    case BLEND_TYPE::DEFAULT:
        rt.BlendEnable = FALSE;
        rt.LogicOpEnable = FALSE;
        rt.SrcBlend = D3D12_BLEND_ONE;
        rt.DestBlend = D3D12_BLEND_ZERO;
        break;
    case BLEND_TYPE::ALPHA_BLEND:
        rt.BlendEnable = TRUE;
        rt.LogicOpEnable = FALSE;
        rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;
    case BLEND_TYPE::ONE_TO_ONE_BLEND:
        rt.BlendEnable = TRUE;
        rt.LogicOpEnable = FALSE;
        rt.SrcBlend = D3D12_BLEND_ONE;
        rt.DestBlend = D3D12_BLEND_ONE;
        break;
    }

    DEVICE->CreateGraphicsPipelineState(&_graphicsPipelineDesc, IID_PPV_ARGS(&_pipelineState));
}

void Shader::CreateComputeShader(const wstring &fileName, const string &name, const string &version) {
    _shaderInfo.shaderType = SHADER_TYPE::COMPUTE;

    CreateShader(fileName, name, version, _csBlob, _computePipelineDesc.CS);
    _computePipelineDesc.pRootSignature = COMPUTE_ROOT_SIGNATURE.Get();

    HRESULT hr = DEVICE->CreateComputePipelineState(&_computePipelineDesc, IID_PPV_ARGS(&_pipelineState));
    assert(SUCCEEDED(hr));
}

void Shader::Update() {
    if (GetShaderType() == SHADER_TYPE::COMPUTE)
        COMPUTE_CMD_LIST->SetPipelineState(_pipelineState.Get());
    else {
        GRAPHICS_CMD_LIST->IASetPrimitiveTopology(_shaderInfo.topology);
        GRAPHICS_CMD_LIST->SetPipelineState(_pipelineState.Get());
    }
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE Shader::GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology) {
    switch (topology) {
    case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
    case D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}

void Shader::CreateVertexShader(const wstring &fileName, const string &name, const string &version) {
    CreateShader(fileName, name, version, _vsBlob, _graphicsPipelineDesc.VS);
}

void Shader::CreatePixelShader(const wstring &fileName, const string &name, const string &version) {
    CreateShader(fileName, name, version, _psBlob, _graphicsPipelineDesc.PS);
}

void Shader::CreateHullShader(const wstring &fileName, const string &name, const string &version) {
    CreateShader(fileName, name, version, _hsBlob, _graphicsPipelineDesc.HS);
}

void Shader::CreateDomainShader(const wstring &fileName, const string &name, const string &version) {
    CreateShader(fileName, name, version, _dsBlob, _graphicsPipelineDesc.DS);
}

void Shader::CreateGeometryShader(const wstring &fileName, const string &name, const string &version) {
    CreateShader(fileName, name, version, _gsBlob, _graphicsPipelineDesc.GS);
}

void Shader::CreateShader(const wstring &fileName, const string &name, const string &version, ComPtr<ID3DBlob> &blob,
                          D3D12_SHADER_BYTECODE &shaderByteCode) {
    uint32 compileFlag = 0;
#ifdef _DEBUG
    compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    wstring path = L"../Resources/Shaders/" + fileName;

    if (FAILED(::D3DCompileFromFile(path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, name.c_str(),
                                    version.c_str(), compileFlag, 0, &blob, &_errBlob))) {
        ::MessageBoxA(nullptr, "Shader Create Failed !", nullptr, MB_OK);
    }

    shaderByteCode = {blob->GetBufferPointer(), blob->GetBufferSize()};
}

