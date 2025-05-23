#include "SPHFluidCommon.hlsli"

StructuredBuffer<int> g_aliveFlagsRead : register(t0);
RWStructuredBuffer<float3> g_velocitiesRW : register(u0);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;

    if (g_aliveFlagsRead[i] == -1)
        return;
   
    float rand = RandRange(i, 0.05, 0.1);
    g_velocitiesRW[i] += float3(0.0, rand, 0.0);
}