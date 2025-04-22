#include "Common.hlsli"

struct PS_IN
{
    float4 posProj : SV_POSITION;
};

struct PS_OUT
{
    float4 pixelColor : SV_Target0;
};

PS_OUT main(PS_IN input)
{   
    PS_OUT output;
    
    output.pixelColor = float4(albedo, 1.0);
    
    return output;
}