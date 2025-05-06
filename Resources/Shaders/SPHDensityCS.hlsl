#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<int> g_aliveFlagsRead : register(t1);
StructuredBuffer<CellRange> g_cellRangesRead : register(t2);

RWStructuredBuffer<float> g_densitiesRW : register(u0);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
        return;

    float3 x_i = g_positionsRead[i];
    
    int3 selfCell = int3((x_i - gridOrigin) / cellSize);
    float density = 0.0;
   
    [unroll]
    for (int z = -1; z <= 1; ++z)
        [unroll]
        for (int y = -1; y <= 1; ++y)
            [unroll]
            for (int x = -1; x <= 1; ++x)
            {
                int3 neighborCell = selfCell + int3(x, y, z);
                uint neighborHash = ComputeHash(neighborCell) & (hashCount - 1);
                CellRange cellRange = g_cellRangesRead[neighborHash];
                
                uint start = clamp(cellRange.startIndex, 0, maxParticles);
                uint end = clamp(cellRange.endIndex, 0, maxParticles);
                
                [loop]
                for (uint j = start; j < end; ++j)
                {
                    int alive_j = (g_aliveFlagsRead[j] != -1) ? 1 : 0;
                    float3 x_j = g_positionsRead[j];
                    int3 cell_j = int3((x_j - gridOrigin) / cellSize);
                    int match = (neighborCell.x == cell_j.x && neighborCell.y == cell_j.y && neighborCell.z == cell_j.z) ? 1 : 0;
                    
                    float3 x_ij = x_i - x_j;
                    float r2 = dot(x_ij, x_ij);
                    
                    density += alive_j * match * (mass * Poly6Kernel2D(r2, h));
                }
            }
   
    g_densitiesRW[i] = density;
}
