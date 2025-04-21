#pragma once

#include "Object.h"

enum class SHADER_TYPE : uint8 {
    DEFERRED,
    FORWARD,
    COMPUTE,
};

enum class RASTERIZER_TYPE : uint8 {
    // SOLID + [...]
    CULL_NONE,
    CULL_FRONT,
    CULL_BACK,
    // WIREFRAME + CULL_NONE
    WIREFRAME,
};

enum class DEPTH_STENCIL_TYPE : uint8 {
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    NO_DEPTH_TEST,          // ���� �׽�Ʈ(X) + ���� ���(O)
    NO_DEPTH_TEST_NO_WRITE, // ���� �׽�Ʈ(X) + ���� ���(X)
    LESS_NO_WRITE,          // ���� �׽�Ʈ(O) + ���� ���(X)
};

enum class BLEND_TYPE : uint8 {
    DEFAULT,
    ALPHA_BLEND,
    ONE_TO_ONE_BLEND,
    END,
};

struct ShaderInfo {
    SHADER_TYPE shaderType = SHADER_TYPE::FORWARD;
    RASTERIZER_TYPE rasterizerType = RASTERIZER_TYPE::CULL_BACK;
    DEPTH_STENCIL_TYPE depthStencilType = DEPTH_STENCIL_TYPE::LESS;
    BLEND_TYPE blendType = BLEND_TYPE::DEFAULT;
    D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
};

struct ShaderArg {
    string vs = "main";
    string ps = "main";
    string hs;
    string ds;
    string gs;
};

class Shader : public Object {
  public:
    Shader();
    virtual ~Shader();

    void CreateGraphicsShader(const wstring &path, ShaderInfo info = ShaderInfo(), ShaderArg arg = ShaderArg());
    void CreateComputeShader(const wstring &path, const string &name, const string &version);

    void Update();

    SHADER_TYPE GetShaderType() { return _shaderInfo.shaderType; }

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType(D3D_PRIMITIVE_TOPOLOGY topology);

  public:
    void CreateVertexShader(const wstring &path, const string &name = "main", const string &version = "vs_5_0");
    void CreatePixelShader(const wstring &path, const string &name = "main", const string &version = "ps_5_0");
    void CreateHullShader(const wstring &path, const string &name = "main", const string &version = "hs_5_0");
    void CreateDomainShader(const wstring &path, const string &name = "main", const string &version = "ds_5_0");
    void CreateGeometryShader(const wstring &path, const string &name = "main", const string &version = "gs_5_0");

  private:
    void CreateShader(const wstring &path, const string &name, const string &version, ComPtr<ID3DBlob> &blob,
                      D3D12_SHADER_BYTECODE &shaderByteCode);

  private:
    ShaderInfo _shaderInfo;
    ComPtr<ID3D12PipelineState> _pipelineState;

    // Graphics Shader
    ComPtr<ID3DBlob> _vsBlob;
    ComPtr<ID3DBlob> _hsBlob;
    ComPtr<ID3DBlob> _dsBlob;
    ComPtr<ID3DBlob> _gsBlob;
    ComPtr<ID3DBlob> _psBlob;
    ComPtr<ID3DBlob> _errBlob;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC _graphicsPipelineDesc = {};

    // Compute Shader
    ComPtr<ID3DBlob> _csBlob;
    D3D12_COMPUTE_PIPELINE_STATE_DESC _computePipelineDesc = {};
};
