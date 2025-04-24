#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<float3> g_velocitiesRead : register(t1);
StructuredBuffer<float> g_densitiesRead : register(t2);
StructuredBuffer<int> g_aliveFlagsRead : register(t3);
StructuredBuffer<CellRange> g_cellRangesRead : register(t4);

RWStructuredBuffer<float3> g_predPositionsRW : register(u0);
RWStructuredBuffer<float3> g_predVelocitiesRW : register(u1);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;

    if (g_aliveFlagsRead[i] == -1)
        return;
    
    float3 x_i = g_positionsRead[i];
    float3 v_i = g_velocitiesRead[i];
    int3 selfCell = int3(ceil((x_i - gridOrigin) / cellSize));
    float3 viscosityForce = float3(0.0, 0.0, 0.0);
    float3 gravity = float3(0.0, -9.8, 0.0);
    float3 gravityForce = mass * gravity;
    
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
                    
                    float3 x_j = g_positionsRead[j];
                    
                    int3 cell_j = int3(ceil((x_j - gridOrigin) / cellSize));
                    if (neighborCell.x != cell_j.x || neighborCell.y != cell_j.y || neighborCell.z != cell_j.z)
                        continue;
                    
                    float3 v_j = g_velocitiesRead[j];
                    float rho_j = g_densitiesRead[j];
                    float3 x_ij = x_i - x_j;
                    float3 v_ij = v_i - v_j;
                    float3 v_ji = v_j - v_i;
                    float r2 = dot(x_ij, x_ij);
                    float h2 = h * h;
                    float r_len = sqrt(r2);
                     
                    if (r_len >= h || r_len < h * 0.01)
                        continue;
                    
                    viscosityForce += mass * viscosity * 2.0 * mass / rho_j * v_ij *
                                      dot(x_ij, GradientSpikyKernel2D(x_ij, r_len, h)) / (dot(x_ij, x_ij) + 0.01 * h * h);
                }
            }
    
    float3 predVelocity = v_i + deltaTime * (viscosityForce + gravityForce) / mass;
    
    g_predVelocitiesRW[i] = predVelocity;
    g_predPositionsRW[i] = x_i + deltaTime * predVelocity;
}