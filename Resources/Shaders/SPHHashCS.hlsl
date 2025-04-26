#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<int> g_aliveFlagsRead : register(t1);

RWStructuredBuffer<uint> g_hashesRW : register(u0);
RWStructuredBuffer<CellRange> g_cellRangesRW : register(u1);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
    {
        g_hashesRW[i] = hashCount - 1;
    }
    else
    {
        int3 cell = int3(floor((g_positionsRead[i] - gridOrigin) / cellSize));

        g_hashesRW[i] = ComputeHash(cell) & (hashCount - 1); // mod
    }
    
    // CellRange 초기화 (주의 해쉬카운트와 maxParticles가 같아야함. 구현 간단화)
    g_cellRangesRW[i].startIndex = 0;
    g_cellRangesRW[i].endIndex = 0;
}