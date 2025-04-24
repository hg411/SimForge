cbuffer SPHFluidParams : register(b3)
{
    uint maxParticles;
    float deltaTime;
    float totalTime;
    float pressureCoeff;
    float nearPressureCoeff;
    float viscosity;
    float density0; // Target Density
    float mass;
    float h; // Smoothing Length
    
    float3 boxCenter;
    float boxWidth;
    float boxHeight;
    float boxDepth;
    
    float cellSize;
    uint hashCount;
    
    float3 gridOrigin;
    
    uint addCount;
    float radius;
    float surfaceCoeff;
    float padding2;
}

StructuredBuffer<float3> g_positionsRead : register(t0);
StructuredBuffer<float3> g_velocitiesRead : register(t1);
StructuredBuffer<int> g_aliveFlagsRead : register(t2);

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
    float life : PSIZE0;
    float radius : PSIZE1;
    int alive : ALIVE;
};

float3 GetColorFromSpeed(float speed, float minSpeed, float maxSpeed)
{
    float t = (speed - minSpeed) / (maxSpeed - minSpeed);
    t = clamp(t, 0.0f, 1.0f); // 0~1 범위로 제한

    // 속도를 기반으로 색상 보간 (Blue → Green → Red)
    float3 color;
    if (t < 0.5f)
    {
        // Blue → Green (0, 0, 1) → (0, 1, 0)
        float blend = t * 2.0f;
        color = float3(0.0, blend, 1.0 - blend);
    }
    else
    {
        // Green → Red (0, 1, 0) → (1, 0, 0)
        float blend = (t - 0.5) * 2.0;
        color = float3(blend, 1.0 - blend, 0.0);
    }

    return color;
}

// VSInput이 없이 vertexID만 사용
VS_OUT main(uint vertexID : SV_VertexID)
{   
    VS_OUT output;
    
    float3 posWorld = g_positionsRead[vertexID];
    float3 velocity = g_velocitiesRead[vertexID];
    
    output.position = float4(posWorld, 1.0);
    output.life = 1.0;
    output.radius = radius;
    
    float maxSpeed = 2.0;
    float minSpeed = 0.0;
    float normalizedSpeed = saturate(length(velocity) / maxSpeed);
    float3 colorSlow = float3(0.0, 0.0, 1.0);
    float3 colorFast = float3(1.0, 0.0, 0.0);
    
    output.color = GetColorFromSpeed(length(velocity), minSpeed, maxSpeed);
    output.alive = g_aliveFlagsRead[vertexID];

    return output;
}
