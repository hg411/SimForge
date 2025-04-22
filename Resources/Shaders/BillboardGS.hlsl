#include "Common.hlsli"

struct GS_IN
{
    float4 pos : SV_POSITION;
    float3 color : COLOR;
    float life : PSIZE0;
    float radius : PSIZE1;
    int alive : ALIVE;
};

struct GS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
    uint primID : SV_PrimitiveID;
};

[maxvertexcount(4)]
void main(point GS_IN input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outputStream)
{
    if (input[0].alive ==  -1)
        return;
    
    float hw = input[0].radius; // halfWidth
    //float3 up = float3(0.0, 1.0, 0.0);
    //float3 right = float3(1.0, 0.0, 0.0);

    GS_OUT output;
    output.color = input[0].color;
    
    float4 centerVS = mul(float4(input[0].pos.xyz, 1.0), g_matView);
    
    //output.pos = float4(centerVS.xyz - hw * right - hw * up, 1.0);
    //output.pos = mul(output.pos, g_matProjection);
    //output.uv = float2(0.0, 1.0);
    //output.primID = primID;

    //outputStream.Append(output);

    //output.pos = float4(centerVS.xyz - hw * right + hw * up, 1.0);
    //output.pos = mul(output.pos, g_matProjection);
    //output.uv = float2(0.0, 0.0);
    //output.primID = primID;
    
    //outputStream.Append(output);

    //output.pos = float4(centerVS.xyz + hw * right - hw * up, 1.0);
    //output.pos = mul(output.pos, g_matProjection);
    //output.uv = float2(1.0, 1.0);
    //output.primID = primID;
    
    //outputStream.Append(output);

    //output.pos = float4(centerVS.xyz + hw * right + hw * up, 1.0);
    //output.pos = mul(output.pos, g_matProjection);
    //output.uv = float2(1.0, 0.0);
    //output.primID = primID;

    //outputStream.Append(output);
    
    float3 up = g_camUp;
    float3 right = g_camRight;
    
    output.pos = float4(input[0].pos.xyz - hw * right - hw * up, 1.0);
    output.pos = mul(output.pos, g_matVP);
    output.uv = float2(0.0, 1.0);
    output.primID = primID;

    outputStream.Append(output);
    
    output.pos = float4(input[0].pos.xyz - hw * right + hw * up, 1.0);
    output.pos = mul(output.pos, g_matVP);
    output.uv = float2(0.0, 0.0);
    output.primID = primID;
    
    outputStream.Append(output);

    output.pos = float4(input[0].pos.xyz + hw * right - hw * up, 1.0);
    output.pos = mul(output.pos, g_matVP);
    output.uv = float2(1.0, 1.0);
    output.primID = primID;
    
    outputStream.Append(output);

    output.pos = float4(input[0].pos.xyz + hw * right + hw * up, 1.0);
    output.pos = mul(output.pos, g_matVP);
    output.uv = float2(1.0, 0.0);
    output.primID = primID;

    outputStream.Append(output);

    outputStream.RestartStrip(); // Strip을 다시 시작
}