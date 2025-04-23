#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<float3> g_predPositionsRead : register(t1);
StructuredBuffer<int> g_aliveFlagsRead : register(t2);
StructuredBuffer<CellRange> g_cellRangesRead : register(t3);

RWStructuredBuffer<float> g_densitiesRW : register(u0);
RWStructuredBuffer<float> g_pressuresRW : register(u1);
RWStructuredBuffer<float> g_nearPressuresRW : register(u2);

[numthreads(256, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
        return;

    float3 x_i = g_predPositionsRead[i];
    
    int3 selfCell = int3(ceil((g_positionsRead[i] - gridOrigin) / cellSize));
    float density = 0.0;
    float nearDensity = 0.0;
   
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
                    if (g_aliveFlagsRead[j] == -1)
                        continue;
                    
                    float3 x_j = g_predPositionsRead[j];
                    
                    int3 cell_j = int3(ceil((g_positionsRead[j] - gridOrigin) / cellSize));
                    if (neighborCell.x != cell_j.x || neighborCell.y != cell_j.y || neighborCell.z != cell_j.z)
                        continue;

                    float3 x_ij = x_i - x_j;
                    float r2 = dot(x_ij, x_ij);
                    float h2 = h * h;
                    
                    if (r2 >= h2)
                        continue;
                    
                    density += mass * Poly6Kernel3D(r2, h);
                    
                    if (i == j)
                        continue;

                    float q = 1.0 - sqrt(r2) / h;
                    nearDensity += q * q * q;
                }
            }
   
    g_densitiesRW[i] = density;
    
    float k = density * radius * radius / (2.0 * density0 * deltaTime * deltaTime) * pressureCoeff;
    
    g_pressuresRW[i] = max(k * (density - density0), 0.0);
    //g_pressuresRW[i] = k * (density - density0);
    g_nearPressuresRW[i] = nearPressureCoeff * nearDensity;
}