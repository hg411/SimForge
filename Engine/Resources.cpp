#include "pch.h"
#include "Resources.h"
#include "Shader.h"
#include "Mesh.h"

void Resources::CreateLineShader() {
    ShaderInfo info = {
        SHADER_TYPE::FORWARD,
        RASTERIZER_TYPE::CULL_NONE, // 작동 안함.(D3D_PRIMITIVE_TOPOLOGY_LINELIST)
        DEPTH_STENCIL_TYPE::LESS,
        BLEND_TYPE::DEFAULT,
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        }};

    ShaderArg arg = {"main", "", "", "", "main"};

    shared_ptr<Shader> shader = make_shared<Shader>();
    shader->CreateVertexShader(L"BasicVS.hlsl");
    shader->CreatePixelShader(L"BasicPS.hlsl");
    shader->CreateGraphicsShader(info);

    Add<Shader>(L"LineShader", shader);
}

shared_ptr<Mesh> Resources::LoadRectangleLineMesh(const float width, const float height) {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;

    vector<Vertex> vec(8);

    vec[0] = Vertex(Vec3(-halfW, -halfH, 0.0f));
    vec[1] = Vertex(Vec3(+halfW, -halfH, 0.0f));

    vec[2] = Vertex(Vec3(+halfW, -halfH, 0.0f));
    vec[3] = Vertex(Vec3(+halfW, +halfH, 0.0f));

    vec[4] = Vertex(Vec3(+halfW, +halfH, 0.0f));
    vec[5] = Vertex(Vec3(-halfW, +halfH, 0.0f));

    vec[6] = Vertex(Vec3(-halfW, +halfH, 0.0f));
    vec[7] = Vertex(Vec3(-halfW, -halfH, 0.0f));

    shared_ptr<Mesh> mesh = make_shared<Mesh>();
    mesh->Create(vec);

    return mesh;
}
