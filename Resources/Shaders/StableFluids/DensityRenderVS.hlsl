struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// VSInput이 없이 vertexID만 사용
VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT output;
    
    // 3개의 정점으로 fullscreen triangle (삼각형 하나로 화면 전체 덮기)
    float2 positions[3] =
    {
        float2(-1.0, -1.0),
        float2(-1.0, 3.0),
        float2(3.0, -1.0)
    };

    float2 uvs[3] =
    {
        float2(0.0, 1.0),
        float2(0.0, -1.0),
        float2(2.0, 1.0)
    };

    output.pos = float4(positions[vertexID], 0.0, 1.0);
    output.uv = uvs[vertexID];
    
    return output;
}
