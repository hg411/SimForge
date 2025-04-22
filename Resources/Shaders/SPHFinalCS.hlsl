#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_predPositionsRead : register(t0);
StructuredBuffer<float3> g_predVelocitiesRead : register(t1);
StructuredBuffer<int> g_aliveFlagsRead : register(t2);

RWStructuredBuffer<float3> g_positionsRW : register(u0);
RWStructuredBuffer<float3> g_velocitiesRW : register(u1);

[numthreads(256, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
        return;
    
    float3 position = g_predPositionsRead[i];
    float3 velocity = g_predVelocitiesRead[i];

    g_positionsRW[i] = position;
    g_velocitiesRW[i] = velocity;
}