#ifndef __SPHCOMMON_HLSLI__
#define __SPHCOMMON_HLSLI__

#include "Constants.hlsli"

#define NUM_THREADS_X 64

struct CellRange
{
    uint startIndex;
    uint endIndex;
};

cbuffer SPHFluidParams : register(b0)
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

cbuffer Const : register(b1)
{
    // https://en.wikipedia.org/wiki/Bitonic_sorter Example Code
    uint k;
    uint j;
}

// Computes hash value for given cell coordinates.
uint ComputeHash(int3 cell)
{
    return ((uint(cell.x) * 73856093) ^ (uint(cell.y) * 19349663) ^ (uint(cell.z) * 83492791));
}

float GradKernelFunction(const float q)
{
    float coeff = 3.0 / (2.0 * 3.141592);
    
    if (q < 1.0)
        return coeff * (-2.0 * q + 1.5 * q * q);
    else if (q < 2.0)
        return coeff * -0.5 * (2.0 - q) * (2.0 - q);
    else // q >= 2.0f
        return 0.0f;
}

float KernelFunction(const float q)
{
    float coeff = 3.0 / (2.0 * 3.141592);
    
    if (q < 1.0)
        return coeff * (2.0 / 3.0 - q * q + 0.5 * q * q * q);
    else if (q < 2.0)
        return coeff * pow(2.0 - q, 3.0) / 6.0;
    else // q >= 2.0f
        return 0.0f;
}

// Poly6 Kernel: use for density.
float Poly6Kernel(float r_len, float h, int dimension)
{
    float result = 0.0;
    float h2 = h * h;
    float h4 = h2 * h2;
    float h8 = h4 * h4;
    float r2 = r_len * r_len;
    float coeff;

    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = 4.0 / (PI * h8); // 2D normalization
    }
    else // dimension == 3
    { 
        coeff = 315.0 / (64.0 * PI * h8 * h); // 3D normalization
    }

    float h2_r2 = h2 - r2;
    
    result = coeff * h2_r2 * h2_r2 * h2_r2;

    return result;
}

float Poly6Kernel2D(float r2, float h2)
{
    float result = 0.0;
    float h4 = h2 * h2;
    float h8 = h4 * h4;
    
    float coeff = 4.0 / (PI * h8); // 2D normalization

    float h2_r2 = h2 - r2;
    
    if (r2 < h2)
    {
        result = coeff * h2_r2 * h2_r2 * h2_r2;
    }

    return result;
}

float Poly6Kernel3D(float r2, float h)
{
    float result = 0.0;
    float h2 = h * h;
    float h4 = h2 * h2;
    float h9 = h4 * h4 * h;
    
    float coeff = 315.0 / (64.0 * PI * h9); // 3D normalization

    float h2_r2 = h2 - r2;
    
    if (r2 < h2)
    {
        result = coeff * h2_r2 * h2_r2 * h2_r2;
    }

    return result;
}

float3 GradientPoly6Kernel(float3 r_vec, float r_len, float h, int dimension)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float h4 = h2 * h2;
    float h8 = h4 * h4;
    float r2 = r_len * r_len;
    float coeff;
    
    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = -24.0 / (PI * h8); // 2D normalization
    }
    else // dimension == 3
    {
        coeff = -945.0 / (32.0 * PI * h8 * h);
    }
    
    float h2_r2 = h2 - r2;
    if (r_len < h)
    {
        result = coeff * h2_r2 * h2_r2 * r_vec;
    }
    
    return result;
}

float3 GradientPoly6Kernel2D(float3 r_vec, float r_len, float h)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float h4 = h2 * h2;
    float h8 = h4 * h4;
    float r2 = r_len * r_len;
    float coeff;
    
    coeff = -24.0 / (PI * h8); // 2D normalization
    
    float h2_r2 = h2 - r2;
    if (r_len < h)
    {
        result = coeff * h2_r2 * h2_r2 * r_vec;
    }
    
    return result;
}

float LaplacianPoly6Kernel2D(float r2, float h2)
{
    float result = 0.0;
    float h4 = h2 * h2;
    float h8 = h4 * h4;
    
    float coeff = -24.0 / (PI * h8);
    
    if (r2 < h2)
    {
        result = coeff * (h2 - r2) * (3.0 * h2 - 7.0 * r2);
    }
    
    return result;
}

// Spiky Kernel: use for pressure forces.
float SpikyKernel(float r_len, float h, int dimension)
{
    float result = 0.0;
    float h2 = h * h;
    float coeff;

    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = 10.0 / (PI * h2 * h2 * h); // 2D normalization
    }
    else // dimension == 3
    { 
        coeff = 15.0 / (PI * h2 * h2 * h2); // 3D normalization
    }

    float h_r = h - r_len;
    if (r_len < h)
    {
        result = coeff * h_r * h_r * h_r;
    }

    return result;
}

float3 GradientSpikyKernel(float3 r_vec, float r_len, float h, int dimension)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float coeff;
    
    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = -30.0 / (PI * h2 * h2 * h); // 2D normalization
    }
    else // dimension == 3
    { 
        coeff = -45.0 / (PI * h2 * h2 * h2); // 3D normalization
    }
    
    float h_r = h - r_len;
    if (r_len < h && r_len > h * 1e-3)
    {
        result = coeff * h_r * h_r * (r_vec / r_len);
    }
    
    return result;
}

float3 GradientSpikyKernel2D(float3 r_vec, float r_len, float h)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float h5 = h2 * h2 * h;
    float coeff;
    
    coeff = -30.0 / (PI * h5); // 2D normalization
    
    float h_r = h - r_len;

    if (r_len < h)
    {
        result = coeff * h_r * h_r * (r_vec / r_len);
    }
    
    return result;
}

float3 GradientSpikyKernel3D(float3 r_vec, float r_len, float h)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float h6 = h2 * h2 * h2;
    float coeff;
    
    coeff = -45.0 / (PI * h6); // 3D normalization
    
    float h_r = h - r_len;

    if (r_len < h)
    {
        result = coeff * h_r * h_r * (r_vec / r_len);
    }
    
    return result;
}

// Viscosity Kernel: Laplacian is always positive.
float ViscosityKernel(float r_len, float h, int dimension)
{
    float result = 0.0;
    float h2 = h * h;
    float h3 = h2 * h;
    float r2 = r_len * r_len;
    float r3 = r2 * r_len;
    float coeff;

    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = 10.0 / (3.0 * PI * h2); // 2D normalization
    }
    else
    { // dimension == 3
        coeff = 15.0 / (2.0 * PI * h2 * h); // 3D normalization
    }

    if (r_len < h)
    {
        result = coeff * (-r3 / (2.0 * h3) + r2 / h2 + h / (2.0 * r_len) - 1.0);
    }

    return result;
}

float3 GradientViscosityKernel(float3 r_vec, float r_len, float h, int dimension)
{
    float3 result = float3(0.0, 0.0, 0.0);
    float h2 = h * h;
    float h3 = h2 * h;
    float coeff;
    
    if (dimension == 2)
    {
        coeff = -10.0 / (PI * h2);
    }
    else
    {
        coeff = -15.0 / (2.0 * PI * h3);
    }
    
    float r3 = r_len * r_len * r_len;
    
    if (r_len < h)
    {
        result = coeff * r_vec * (-3.0 * r_len / (2.0 * h3) + 2.0 / h2 - h / (2.0 * r3));

    }
    
    return result;
}

float LaplacianViscosityKernel(float r_len, float h, int dimension)
{
    float result = 0.0;
    float h2 = h * h;
    float coeff;
    
    // Calculate the normalization constants for 2D and 3D.
    if (dimension == 2)
    {
        coeff = 20.0 / (PI * h2 * h2 * h); // 2D normalization
    }
    else // dimension == 3
    {
        coeff = 45.0 / (2.0 * PI * h2 * h2 * h2); // 3D normalization
    }
    
    if (r_len < h)
    {
        result = coeff * (h - r_len);
    }
    
    return result;
}


float LaplacianViscosityKernel2D(float r_len, float h)
{
    float result = 0.0;
    float h2 = h * h;
    float coeff;
    
    coeff = 20.0 / (PI * h2 * h2 * h); // 2D normalization
    
    if (r_len < h)
    {
        result = coeff * (h - r_len);
    }
    
    return result;
}

float LaplacianViscosityKernel3D(float r_len, float h)
{
    float result = 0.0;
    float h2 = h * h;
    float h6 = h2 * h2 * h2;
    float coeff;
    
    coeff = 45.0 / (PI * h6); // 3D normalization
    
    if (r_len < h)
    {
        result = coeff * (h - r_len);
    }
    
    return result;
}

// HLSL: 2D 좌표를 입력받아 0.0 ~ 1.0 범위의 난수를 반환
float Rand(float2 co)
{
    // dot product와 sin을 사용해 해시값 생성 후, fract로 소수부만 추출
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(co, float2(a, b));
    float sn = sin(dt);
    return frac(sn * c);
}

float RandRange(float2 co, float minVal, float maxVal)
{
    return minVal + (maxVal - minVal) * Rand(co);
}

#endif // __SPHCOMMON_HLSLI__