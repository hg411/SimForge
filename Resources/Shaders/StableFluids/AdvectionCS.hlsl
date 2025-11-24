// Advect Velocity and Density

Texture2D<float2> velocityTemp : register(t0);
Texture2D<float4> densityTemp : register(t1);

RWTexture2D<float2> velocity : register(u0);
RWTexture2D<float4> density : register(u1);

// Repeated Boundary
SamplerState linearWrapSS : register(s0);
SamplerState linearClampSS : register(s1);
SamplerState pointWrapSS : register(s2);

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
    float2 dx = float2(1.0 / width, 1.0 / height);
    
    float2 pos = (dtID.xy + 0.5) * dx; // 이 쓰레드가 처리하는 셀의 중심
    
    // 1. velocityTemp로부터 속도 샘플링해오기
    float2 vel = velocityTemp.SampleLevel(linearWrapSS, pos, 0);
    
    //float2 vel = velocityTemp.Load(int3(dtID.xy, 0));
    
    // 2. 그 속도를 이용해서 역추적 위치 계산
    // dx를 통해 해상도 보정을 맞춰줘도 가로 세로 해상도가 다를때 가로세로 속도차이 발생.
    // UV 사각형의 가로세로 길이가 달라서 Linear Interpolation 에서 오차 발생..? 
    float advScale = 1024.0;
    float2 posBack = pos - vel * advScale * dt * dx;

    // 3. 그 위치에서 샘플링 해오기
    if (wallBoundaryCondition == 3)
    {
        velocity[dtID.xy] = velocityTemp.SampleLevel(linearWrapSS, posBack, 0) * 0.999;
        density[dtID.xy] = densityTemp.SampleLevel(linearWrapSS, posBack, 0) * 0.999;
    }
    else
    {
        velocity[dtID.xy] = velocityTemp.SampleLevel(linearClampSS, posBack, 0) * 0.999;
        density[dtID.xy] = densityTemp.SampleLevel(linearClampSS, posBack, 0) * 0.999;
    }
    
    //velocity[dtID.xy] = velocityTemp.SampleLevel(linearWrapSS, posBack, 0);
    //density[dtID.xy] = densityTemp.SampleLevel(linearWrapSS, posBack, 0);
}
