Texture2D<float> vorticity : register(t0);
RWTexture2D<float2> velocity : register(u0);

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

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    velocity.GetDimensions(width, height);

    if (wallBoundaryCondition == 3) // Periodic
    {
        // Vorticity confinement
        uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
        uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
        uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
        uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);

        //float2 eta = float2((abs(vorticity[right]) - abs(vorticity[left])) / (2.0 * dx.x), 
        //                    (abs(vorticity[up]) - abs(vorticity[down])) / (2.0 * dx.y));
    
        float2 eta = float2((abs(vorticity[right]) - abs(vorticity[left])) * 0.5,
                        (abs(vorticity[up]) - abs(vorticity[down])) * 0.5);

        if (length(eta) < 1e-5)
            return;
    
        float3 psi = float3(normalize(eta), 0.0);
        float3 omega = float3(0.0, 0.0, vorticity[dtID.xy]);
        
        velocity[dtID.xy] += vorticityScale * cross(psi, omega).xy * dt;
    }
    else
    {
        if (dtID.x > 1 && dtID.y > 1 && dtID.x < width - 2 && dtID.y < height - 2)
        {
            float2 eta = float2((abs(vorticity[dtID.xy + uint2(1, 0)]) - abs(vorticity[dtID.xy - uint2(1, 0)])) * 0.5,
                                (abs(vorticity[dtID.xy + uint2(0, 1)]) - abs(vorticity[dtID.xy - uint2(0, 1)])) * 0.5);

            if (length(eta) < 1e-5)
                return;
    
            float3 psi = float3(normalize(eta), 0.0);
            float3 omega = float3(0.0, 0.0, vorticity[dtID.xy]);
            
            velocity[dtID.xy] += vorticityScale * cross(psi, omega).xy * dt;
        }
    }
    
}
