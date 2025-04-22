#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<int> g_aliveFlagsRead : register(t1);

RWStructuredBuffer<uint> g_hashesRW : register(u0);

[numthreads(256, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
    {
        g_hashesRW[i] = hashCount - 1;
        
        return;
    }
    
    int3 cell = int3(ceil((g_positionsRead[i] - gridOrigin) / cellSize));

    g_hashesRW[i] = ComputeHash(cell) & (hashCount - 1); // mod
}