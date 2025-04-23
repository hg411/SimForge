#include "SPHFluidCommon.hlsli"

RWStructuredBuffer<float3> g_positionsRW : register(u0);
RWStructuredBuffer<float3> g_velocitiesRW : register(u1);
RWStructuredBuffer<int> g_aliveFlagsRW : register(u2);

groupshared uint localAddCount;

[numthreads(256, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID, int3 gtID : SV_GroupThreadID, int3 gID : SV_GroupID)
{
    uint i = dtID.x;
    
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
        g_velocitiesRW[i] = float3(3.0, 0.0, 0.0);
    }
}