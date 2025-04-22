#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState g_linearWrapSampler : register(s0);
SamplerState g_linearClampSampler : register(s1);
SamplerState g_pointWrapSampler : register(s2);
SamplerState g_pointClampSampler : register(s3);
SamplerState g_maxAnisoWrapSampler : register(s4);
SamplerState g_maxAnisoClampSampler : register(s5);

cbuffer GLOBAL_PARAMS : register(b0)
{
    matrix g_matView;
    matrix g_matProj;
    matrix g_matVP;
    matrix g_matInvProj;
    float3 g_eyeWorld;
    float padding1;
    
    float3 g_camUp;
    float padding2;
    
    float3 g_camRight;
    float padding3;
}

cbuffer TRANSFORM_PARAMS : register(b1)
{
    matrix matWorld;
    matrix matInvTranspose;
};

cbuffer MATERIAL_PARAMS : register(b2)
{
    float3 albedo;
    float metallic;
    float roughness;
    float3 padding4;
}

#endif // __COMMON_HLSLI__