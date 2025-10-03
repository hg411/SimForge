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
}

// WRap, Clamp �Լ�
int WrapIndex(int i, int N)
{
    return (i % N + N) % N;
}

int ClampIndex(int i, int N)
{
    return (i < 0) ? 0 : (i >= N ? N - 1 : i);
}

// ��ǥ ���� �Լ�
int2 FixIndex(int2 ij, int2 WH, uint wrap)
{
    if (wrap)
        return int2(WrapIndex(ij.x, WH.x), WrapIndex(ij.y, WH.y));
    else
        return int2(ClampIndex(ij.x, WH.x), ClampIndex(ij.y, WH.y));
}

// Lerp �Լ�
float Lerp1(float a, float b, float t)
{ 
    return a + (b - a) * t;
}

float2 Lerp2(float2 a, float2 b, float t)
{
    return a + (b - a) * t;
}

float4 Lerp4(float4 a, float4 b, float t)
{
    return a + (b - a) * t;
}

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    velocity.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);

    float2 pos = (dtID.xy + 0.5) * dx; // �� �����尡 ó���ϴ� ���� �߽�
    //float2 pos = dtID.xy + 0.5; // �� �����尡 ó���ϴ� ���� �߽�
    
    // TODO: 1. velocityTemp�κ��� �ӵ� ���ø��ؿ���
    float2 vel = velocityTemp.SampleLevel(linearWrapSS, pos, 0);
    
    //float2 vel = velocityTemp.Load(int3(dtID.xy, 0));
    
    // TODO: 2. �� �ӵ��� �̿��ؼ� ������ ��ġ ���
    float advScale = 1000.0;
    float2 posBack = pos - vel * advScale * dt * dx;

    // TODO: 3. �� ��ġ���� ���ø� �ؿ���
    velocity[dtID.xy] = velocityTemp.SampleLevel(linearWrapSS, posBack, 0);
    density[dtID.xy] = densityTemp.SampleLevel(linearWrapSS, posBack, 0);
}
