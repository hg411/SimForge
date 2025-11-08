RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);
RWTexture2D<int> boundaryCondition : register(u2);

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

// https://en.wikipedia.org/wiki/Smoothstep
float smootherstep(float x, float edge0 = 0.0f, float edge1 = 1.0f)
{
  // Scale, and clamp x to 0..1 range
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);

    return x * x * x * (3 * x * (2 * x - 5) + 10.0f);
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    density.GetDimensions(width, height);

    // 약간의 Dissipation
    density[dtID.xy] = max(0.0, density[dtID.xy] - 0.001);
    
    // unsigned int라서 마우스 입력이 없을 경우 CPU 코드에서 i = -1
    // 오버플로우로 인해 width 보다 큰 값으로 설정
    int radius = 30;
    if (i < width)
    {
        float dist = length(float2(dtID.xy) - float2(i, j)) / radius;
        float scale = smootherstep(1.0 - dist);

        velocity[dtID.xy] += sourcingVelocity * scale;
        //velocity[dtID.xy] += float2(-0.0, -0.1) * scale; 
        density[dtID.xy] += sourcingDensity * scale;
    }
    
    //***************************
    // Boundary Condition Setting
    //***************************
    
    // 0: Normal Cell
    // 1: Dirichlet Boundary Condition
    // 2: Neumann Boundary Condition
    // 3: Periodic Boundary Condition
    
    if (dtID.x == 0 || dtID.y == 0 || dtID.x == width - 1 || dtID.y == height - 1)
    {
        boundaryCondition[dtID.xy] = wallBoundaryCondition;
    }
    else
    {
        boundaryCondition[dtID.xy] = 0; // Normal Cell
    }
}
