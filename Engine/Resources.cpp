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

shared_ptr<Mesh> Resources::LoadCubeLineMesh(const float width, const float height, const float depth) {
    float halfW = width * 0.5f;
    float halfH = height * 0.5f;
    float halfD = depth * 0.5f;

    vector<Vertex> vec(24);

    vec[0] = Vertex(Vec3(-halfW, -halfH, +halfD));
    vec[1] = Vertex(Vec3(+halfW, -halfH, +halfD));

    vec[2] = Vertex(Vec3(+halfW, -halfH, +halfD));
    vec[3] = Vertex(Vec3(+halfW, +halfH, +halfD));

    vec[4] = Vertex(Vec3(+halfW, +halfH, +halfD));
    vec[5] = Vertex(Vec3(-halfW, +halfH, +halfD));

    vec[6] = Vertex(Vec3(-halfW, +halfH, +halfD));
    vec[7] = Vertex(Vec3(-halfW, -halfH, +halfD));

    vec[8] = Vertex(Vec3(-halfW, -halfH, -halfD));
    vec[9] = Vertex(Vec3(+halfW, -halfH, -halfD));

    vec[10] = Vertex(Vec3(+halfW, -halfH, -halfD));
    vec[11] = Vertex(Vec3(+halfW, +halfH, -halfD));

    vec[12] = Vertex(Vec3(+halfW, +halfH, -halfD));
    vec[13] = Vertex(Vec3(-halfW, +halfH, -halfD));

    vec[14] = Vertex(Vec3(-halfW, +halfH, -halfD));
    vec[15] = Vertex(Vec3(-halfW, -halfH, -halfD));

    vec[16] = Vertex(Vec3(-halfW, -halfH, +halfD));
    vec[17] = Vertex(Vec3(-halfW, -halfH, -halfD));

    vec[18] = Vertex(Vec3(+halfW, -halfH, +halfD));
    vec[19] = Vertex(Vec3(+halfW, -halfH, -halfD));

    vec[20] = Vertex(Vec3(+halfW, +halfH, +halfD));
    vec[21] = Vertex(Vec3(+halfW, +halfH, -halfD));

    vec[22] = Vertex(Vec3(-halfW, +halfH, +halfD));
    vec[23] = Vertex(Vec3(-halfW, +halfH, -halfD));

    shared_ptr<Mesh> mesh = make_shared<Mesh>();
    mesh->Create(vec);

    return mesh;
}
