Texture2D<float4> density : register(t0);

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// 단순 출력
float4 main(PS_IN input) : SV_Target
{
    return float4(0, 0, 0, 0);
}
