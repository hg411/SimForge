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

// 일반적으로 Sprite는 텍스춰를 많이 사용합니다.
// 이 예제처럼 수식으로 패턴을 만들 수도 있습니다.
float4 main(PixelShaderInput input) : SV_TARGET
{
    //float dist = length(float2(0.5, 0.5) - input.uv);
    
    //float scale = stepFunction(dist);
    
    //float4 color = float4(input.color.rgb, 1.0) * scale;
    
    //return color;
    
    float2 uv = input.uv;
    float2 centeredUV = uv * 2.0 - 1.0; // [0,1] → [-1,1]
    float dist = length(centeredUV);

    // 원형 마스크
    if (dist > 1.0)
        discard;

    // Spherical shading (가짜 normal)
    float3 normal = normalize(float3(centeredUV, sqrt(saturate(1.0 - dist * dist))));

    // 조명 방향 (카메라 기준)
    float3 lightDir = normalize(float3(0.4, 0.6, 1.0)); // 원하는 방향 조정

    float lighting = saturate(dot(normal, lightDir));

    // 색상 조합 (기본 색 * 조명)
    float3 baseColor = input.color.rgb;
    float3 shaded = baseColor * (0.3 + 0.7 * lighting); // ambient + diffuse

    return float4(shaded, 1.0);
}
