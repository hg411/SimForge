Texture2D<float> pressure : register(t0);
Texture2D<int> boundaryMap : register(t1);

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
    velocity.GetDimensions(width, height);
    
    if (wallBoundaryCondition == 3) // Periodic
    {
        uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
        uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
        uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
        uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
    
        // 압력의 gradient 계산
        float2 dp = float2(pressure[right] - pressure[left], pressure[up] - pressure[down]);
        float2 gradient = dp * 0.5;

        // velocity 업데이트
        velocity[dtID.xy] -= gradient;
    }
    else
    {
        if (boundaryMap[dtID.xy] == 0) // Normal Cell
        {
            float p[4];

            [unroll]
            for (int i = 0; i < 4; i++)
            {
                if (boundaryMap[dtID.xy + offset[i]] == 1) // Dirichlet
                {         
                    p[i] = -pressure[dtID.xy];
                }
                else if (boundaryMap[dtID.xy + offset[i]] == 2) // Neumann
                {
                    p[i] = pressure[dtID.xy];
                }
                else // Normal Cell
                    p[i] = pressure[dtID.xy + offset[i]];
            }

            velocity[dtID.xy] -= 0.5 * float2(p[0] - p[1], p[2] - p[3]);
        }
    }
}