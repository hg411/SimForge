#include "SPHFluidCommon.hlsli"

RWStructuredBuffer<float3> g_positionsRW : register(u0);
RWStructuredBuffer<float3> g_velocitiesRW : register(u1);
RWStructuredBuffer<int> g_aliveFlagsRW : register(u2);

groupshared uint localAddCount;

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID, int3 gtID : SV_GroupThreadID, int3 gID : SV_GroupID)
{
    uint i = dtID.x;
    
    //if (gtID.x == 0)
    //{
    //    localAddCount = addCount;
    //}
    
    //GroupMemoryBarrierWithGroupSync(); // 동기화부분: 모든 쓰레드들이 이부분까지 진행될때까지 기다림.
    
    //if (i >= maxParticles)
    //    return;
    
    //if (g_aliveFlagsRW[i] == -1)
    //{
    //    int originalValue;
        
    //    //InterlockedAdd(g_shared[gID.x].addCount, -1, originalValue);
    //    InterlockedAdd(localAddCount, -1, originalValue);
        
    //    if (originalValue > 0)
    //    {
    //        g_aliveFlagsRW[i] = 1.0;
    //        float rand1 = RandRange(float2(i, totalTime), -0.1, 0.1);
    //        float rand2 = RandRange(float2(rand1, totalTime), -0.1, 0.1);
    //        float rand3 = RandRange(float2(rand2, totalTime), -0.2, 0.2);
            
    //        if (rand2 > 0.0)
    //        {
    //            g_positionsRW[i] = float3(1.0, 0.3, 1.5) + float3(rand1, rand2, rand3);
    //            g_velocitiesRW[i] = float3(-1.0, 0.0, 0.0);
    //        }
    //        else
    //        {
    //            g_positionsRW[i] = float3(-1.0, 0.3, 1.5) + float3(rand1, rand2, rand3);
    //            g_velocitiesRW[i] = float3(1.0, 0.0, 0.0);
    //        }
    //    }
    //}
    
    // Activate
    //if (i >= maxParticles)
    //    return;
    
    uint nx = 128;
    uint ny = 100;
    uint nz = 64; // 819200개 까지 가능
    
    uint x = i % nx;
    uint z = (i / nx) % nz;
    uint y = i / (nx * nz);
    
    float spacing = 2.0 * radius;
    float3 offset = float3(x, y, z) * spacing;
    
    float3 pos = boxCenter - float3(boxWidth - 50.0 * radius, boxHeight - 1.0, boxDepth - 50.0 * radius) * 0.5 + offset;
    
    if (x < nx && y < ny && z < nz)
    {
        g_aliveFlagsRW[i] = 1;
        g_positionsRW[i] = pos;
        g_velocitiesRW[i] = float3(0.0, 0.0, 0.0);
    }
}