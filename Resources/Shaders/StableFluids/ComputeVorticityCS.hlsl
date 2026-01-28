Texture2D<float2> velocity : register(t0);
RWTexture2D<float> vorticity : register(u0);

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

float Vorticity(uint2 center)
{
    float v2_x = velocity[center + uint2(1, 0)].y - velocity[center - uint2(1, 0)].y;
    float v1_y = velocity[center + uint2(0, 1)].x - velocity[center - uint2(0, 1)].x;
    
    return (v2_x - v1_y) * 0.5;
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    vorticity.GetDimensions(width, height);

    if (wallBoundaryCondition == 3) // Periodic
    {
        uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
        uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
        uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
        uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
        
        vorticity[dtID.xy] = (velocity[right].y - velocity[left].y) * 0.5
                       - (velocity[up].x - velocity[down].x) * 0.5;
    }
    else
    {
        if (dtID.x > 1 && dtID.y > 1 && dtID.x < width - 2 && dtID.y < height - 2)
        {
            vorticity[dtID.xy] = Vorticity(dtID.xy);
        }
    }
   
    
    // TODO: vorticity °è»ê    
    //vorticity[dtID.xy] = (velocity[up].y - velocity[down].y) / (2.0 * dx.y)
    //            - (velocity[right].x - velocity[left].x) / (2.0 * dx.x);
    
    //vorticity[dtID.xy] = (velocity[right].y - velocity[left].y) / (2.0 * dx.x)
    //            - (velocity[up].x - velocity[down].x) / (2.0 * dx.y);
    }
