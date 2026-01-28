Texture2D<float2> velocity : register(t0);
Texture2D<int> boundaryMap : register(t1);
RWTexture2D<float> divergence : register(u0);
RWTexture2D<float> pressure : register(u1);
RWTexture2D<float> pressureTemp : register(u2);

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
    int2(1, 0),   // right
    int2(-1, 0),  // left
    int2(0, 1),   // up
    int2(0, -1)   // down
};

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    divergence.GetDimensions(width, height);

    if (wallBoundaryCondition == 3) // Periodic
    {
        uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
        uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
        uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
        uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);

        //float2 du = float2(velocity[right].x - velocity[left].x, velocity[up].y - velocity[down].y);
        //divergence[dtID.xy] = du.x * 0.5 + du.y * 0.5;

        divergence[dtID.xy] = 0.5 * (velocity[right].x - velocity[left].x + velocity[up].y - velocity[down].y);
    
        pressure[dtID.xy] = 0.0;
        pressureTemp[dtID.xy] = 0.0;
    }
    else
    {
        if (boundaryMap[dtID.xy] == 0)
        {
            float div = 0.0;
            
            [unroll]
            for (int i = 0; i < 4; i++)
            {
                if (boundaryMap[dtID.xy + offset[i]] == 1) // Dirichlet
                {
                    div += dot(velocity[dtID.xy].xy, float2(offset[i]));
                }
                if (boundaryMap[dtID.xy + offset[i]] == 2) // Neumann
                {
                    div += dot(2.0 * velocity[dtID.xy + offset[i]].xy - velocity[dtID.xy].xy, float2(offset[i]));
                }
                else // Normal Cell
                {
                    div += dot(velocity[dtID.xy + offset[i]].xy, float2(offset[i]));
                }
            }

            divergence[dtID.xy] = 0.5 * div;
            
            pressure[dtID.xy] = 0.0;
            pressureTemp[dtID.xy] = 0.0;
        }
    }
}