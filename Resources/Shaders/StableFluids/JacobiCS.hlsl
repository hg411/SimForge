Texture2D<float> pressureTemp : register(t0);
Texture2D<float> divergence : register(t1);
Texture2D<int> boundaryMap : register(t2);

RWTexture2D<float> pressureOut : register(u0);

SamplerState pointWrapSS : register(s0);
SamplerState linearWrapSS : register(s1);

cbuffer Consts : register(b0)
{
    float dt;
    float viscosity;
    float2 sourcingVelocity;
    
    float4 sourcingDensity;
    
    uint i;
    uint j;
    float vorticityScale;
    int wallBoundaryCondition;
}

static int2 offset[4] =
{
    int2(1, 0), // right
    int2(-1, 0), // left
    int2(0, 1), // up
    int2(0, -1) // down
};

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{    
    uint width, height;
    pressureOut.GetDimensions(width, height);
        
    if (wallBoundaryCondition == 3) // Periodic
    {
        uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
        uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
        uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
        uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
    
        pressureOut[dtID.xy] = 0.25 * (-divergence[dtID.xy]
                                   + pressureTemp[left]
                                   + pressureTemp[right]
                                   + pressureTemp[up]
                                   + pressureTemp[down]);
    }
    else
    {
        if (boundaryMap[dtID.xy] == 0) // Normal Cell
        {
            float temp = 0.0;
            
            [unroll]
            for (int i = 0; i < 4; i++)
            {
                if (boundaryMap[dtID.xy + offset[i]] == 1) // Dirichlet
                {
                    temp += -pressureTemp[dtID.xy];
                }
                else if (boundaryMap[dtID.xy + offset[i]] == 2) // Neumann
                {
                    temp += pressureTemp[dtID.xy];
                }
                else // Normal Cell
                {
                    temp += pressureTemp[dtID.xy + offset[i]];
                }
            }

            pressureOut[dtID.xy] = (-divergence[dtID.xy] + temp) * 0.25;
        }
    }
}
