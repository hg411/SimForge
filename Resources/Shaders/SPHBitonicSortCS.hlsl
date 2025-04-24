#include "SPHFluidCommon.hlsli"

RWStructuredBuffer<float3> g_positionsRW : register(u0);
RWStructuredBuffer<float3> g_velocitiesRW : register(u1);
RWStructuredBuffer<int> g_aliveFlagsRW : register(u2);
RWStructuredBuffer<uint> g_hashesRW : register(u3);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    uint l = i ^ j;
    
    //if (i >= maxParticles || l >= maxParticles)
    //    return;
    
    if (l > i)
    {
        uint iHash = g_hashesRW[i];
        uint lHash = g_hashesRW[l];
       
        
        if (((i & k) == 0) && (iHash > lHash) || ((i & k) != 0) && (iHash < lHash))
        {
            g_hashesRW[i] = lHash;
            g_hashesRW[l] = iHash;
            
            float3 tempPosition = g_positionsRW[i];
            g_positionsRW[i] = g_positionsRW[l];
            g_positionsRW[l] = tempPosition;
            
            float3 tempVelocity = g_velocitiesRW[i];
            g_velocitiesRW[i] = g_velocitiesRW[l];
            g_velocitiesRW[l] = tempVelocity;
            
            int tempAliveFlag = g_aliveFlagsRW[i];
            g_aliveFlagsRW[i] = g_aliveFlagsRW[l];
            g_aliveFlagsRW[l] = tempAliveFlag;
        }
    }
    
    GroupMemoryBarrierWithGroupSync();
}