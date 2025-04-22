#include "Common.hlsli"

struct VS_IN
{
    float3 posLocal : POSITION;
};

struct PS_IN
{
    float4 posProj : SV_POSITION;
};

PS_IN main(VS_IN input)
{
    PS_IN output;
    
    float4 pos = float4(input.posLocal, 1.0f);
    pos = mul(pos, matWorld);
    pos = mul(pos, g_matVP);

    output.posProj = pos;

    return output;
}
