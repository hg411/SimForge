Texture2D<float4> density : register(t0);

SamplerState linearWrapSampler : register(s0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float3 LinearToneMapping(float3 color)
{
    float3 invGamma = float3(1.0, 1.0, 1.0) / 2.2;

    color = 1.0 * color; // LinearToneMapping
    color = pow(abs(color), invGamma);
    
    return color;
}

// 단순 출력
float4 main(PS_IN input) : SV_Target
{
    float4 output = density.Sample(linearWrapSampler, input.uv);
    
    output.xyz = LinearToneMapping(output.xyz);
    float maximumColor = max(max(output.x, output.y), output.z);
    
    if (maximumColor >= 1.0)
        output.xyz /= maximumColor;
    
    return output;
}
