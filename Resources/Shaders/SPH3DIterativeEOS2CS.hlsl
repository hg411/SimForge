#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<float3> g_predPositionsRead : register(t1);
StructuredBuffer<int> g_aliveFlagsRead : register(t2);
StructuredBuffer<CellRange> g_cellRangesRead : register(t3);
StructuredBuffer<float> g_densitiesRead : register(t4);
StructuredBuffer<float> g_pressuresRead : register(t5);
StructuredBuffer<float> g_nearPressuresRead : register(t6);

RWStructuredBuffer<float3> g_forcesRW : register(u0);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
        return;
    
    float3 x_i = g_predPositionsRead[i];
    float rho_i = g_densitiesRead[i];
    float p_i = g_pressuresRead[i];
    float np_i = g_nearPressuresRead[i];
    
    int3 selfCell = int3(floor((g_positionsRead[i] - gridOrigin) / cellSize));
    float3 forcePressure = float3(0.0, 0.0, 0.0);
    
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
                
                [loop]
                for (uint j = cellRange.startIndex; j < cellRange.endIndex; ++j)
                {
                    if (i == j)
                        continue;
                    
                    if (g_aliveFlagsRead[j] == -1)
                        continue;
                    
                    float3 x_j = g_predPositionsRead[j];
                    
                    int3 cell_j = int3(floor((g_positionsRead[j] - gridOrigin) / cellSize));
                    if (neighborCell.x != cell_j.x || neighborCell.y != cell_j.y || neighborCell.z != cell_j.z)
                        continue;
                    
                    
                    float rho_j = g_densitiesRead[j];
                    float p_j = g_pressuresRead[j];
                    float np_j = g_nearPressuresRead[j];
               
                    float3 x_ij = x_i - x_j;
                    float r_len = length(x_ij);
                    
                    if (r_len >= h || r_len < h * 1e-3)
                        continue;
                    
                    forcePressure -= mass * mass * (p_i / (rho_i * rho_i) + p_j / (rho_j * rho_j)) * GradientSpikyKernel3D(x_ij, r_len, h);
                    
                    // Near Pressure Force
                    float3 dir = x_ij / r_len; // normalize
                    forcePressure -= dir * (np_i + np_j) / 2.0;
                }
            }
    
    g_forcesRW[i] = forcePressure; // force == forcePressure
}