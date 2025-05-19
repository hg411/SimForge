#include "SPHFluidCommon.hlsli"

StructuredBuffer<uint> g_hashesRead : register(t0);
RWStructuredBuffer<CellRange> g_cellRangesRW : register(u0);

[numthreads(NUM_THREADS_X, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    uint iHash = g_hashesRead[i];
    
    if (i == 0)
    {
        g_cellRangesRW[iHash].startIndex = i;
    }
    else if (iHash != g_hashesRead[i - 1])
    {
        g_cellRangesRW[iHash].startIndex = i;
    }
        

    if (i == maxParticles - 1)
    {
        g_cellRangesRW[iHash].endIndex = i + 1;
    }
    else if (iHash != g_hashesRead[i + 1])
    {
        g_cellRangesRW[iHash].endIndex = i + 1;
    }
}