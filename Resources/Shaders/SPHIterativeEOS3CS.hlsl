#include "SPHFluidCommon.hlsli"

StructuredBuffer<float3> g_forcesRead : register(t0);
StructuredBuffer<int> g_aliveFlagsRead : register(t1);

RWStructuredBuffer<float3> g_predPositionsRW : register(u0);
RWStructuredBuffer<float3> g_predVelocitiesRW : register(u1);

[numthreads(256, 1, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
    uint i = dtID.x;
    
    if (i >= maxParticles)
        return;
    
    if (g_aliveFlagsRead[i] == -1)
        return;
    
    float3 forcePressure = g_forcesRead[i];
    
    float3 acceleration = forcePressure / mass;
    float3 velocity = g_predVelocitiesRW[i] + deltaTime * acceleration;
    float3 position = g_predPositionsRW[i] + deltaTime * deltaTime * acceleration;
    
    float3 center = boxCenter;
    float width = boxWidth;
    float height = boxHeight;
    float depth = boxDepth;
    
    float wallTop = center.y + height * 0.5;
    float wallBottom = center.y - height * 0.5;
    float wallBack = center.z + depth * 0.5;
    float wallFront = center.z - depth * 0.5;
    float wallRight = center.x + width * 0.5;
    float wallLeft = center.x - width * 0.5;
    float cor = 0.5;
    if (position.z < wallFront + radius)
    {
        position.z = wallFront + radius;
        velocity.z *= -cor;
    }
        
    if (position.z > wallBack - radius)
    {
        position.z = wallBack - radius;
        velocity.z *= -cor;
    }
    
    if (position.y < wallBottom + radius)
    {
        position.y = wallBottom + radius;
        velocity.y *= -cor;
    }
        
    if (position.y > wallTop - radius)
    {
        position.y = wallTop - radius;
        velocity.y *= -cor;
    }
        
    if (position.x < wallLeft + radius)
    {
        position.x = wallLeft + radius;
        velocity.x *= -cor;
    }

    if (position.x > wallRight - radius)
    {
        position.x = wallRight - radius;
        velocity.x *= -cor;
    }
    
    g_predVelocitiesRW[i] = velocity;
    g_predPositionsRW[i] = position;
}