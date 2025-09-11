struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// VSInput�� ���� vertexID�� ���
VS_OUT main(uint vertexID : SV_VertexID)
{
    VS_OUT output;
    
    // 3���� �������� fullscreen triangle (�ﰢ�� �ϳ��� ȭ�� ��ü ����)
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
