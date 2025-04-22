struct PixelShaderInput
{
    float4 position : SV_POSITION; // not POSITION
    float2 uv : TEXCOORD;
    float3 color : COLOR;
    uint primID : SV_PrimitiveID;
};

float stepFunction(float dist)
{
    if (dist > 0.5)
        return 0.0;
    else 
        return 1.0;
}

// �Ϲ������� Sprite�� �ؽ��縦 ���� ����մϴ�.
// �� ����ó�� �������� ������ ���� ���� �ֽ��ϴ�.
float4 main(PixelShaderInput input) : SV_TARGET
{
    //float dist = length(float2(0.5, 0.5) - input.uv);
    
    //float scale = stepFunction(dist);
    
    //float4 color = float4(input.color.rgb, 1.0) * scale;
    
    //return color;
    
    float2 uv = input.uv;
    float2 centeredUV = uv * 2.0 - 1.0; // [0,1] �� [-1,1]
    float dist = length(centeredUV);

    // ���� ����ũ
    if (dist > 1.0)
        discard;

    // Spherical shading (��¥ normal)
    float3 normal = normalize(float3(centeredUV, sqrt(saturate(1.0 - dist * dist))));

    // ���� ���� (ī�޶� ����)
    float3 lightDir = normalize(float3(0.4, 0.6, 1.0)); // ���ϴ� ���� ����

    float lighting = saturate(dot(normal, lightDir));

    // ���� ���� (�⺻ �� * ����)
    float3 baseColor = input.color.rgb;
    float3 shaded = baseColor * (0.3 + 0.7 * lighting); // ambient + diffuse

    return float4(shaded, 1.0);
}
