Texture2D<float2> velocity : register(t0);
RWTexture2D<float> vorticity : register(u0);

[numthreads(32, 32, 1)]
void main(int3 gID : SV_GroupID, int3 gtID : SV_GroupThreadID,
          uint3 dtID : SV_DispatchThreadID)
{
    uint width, height;
    vorticity.GetDimensions(width, height);
    float2 dx = float2(1.0 / width, 1.0 / height);

    uint2 left = uint2(dtID.x == 0 ? width - 1 : dtID.x - 1, dtID.y);
    uint2 right = uint2(dtID.x == width - 1 ? 0 : dtID.x + 1, dtID.y);
    uint2 up = uint2(dtID.x, dtID.y == height - 1 ? 0 : dtID.y + 1);
    uint2 down = uint2(dtID.x, dtID.y == 0 ? height - 1 : dtID.y - 1);
    
    // TODO: vorticity °è»ê    
    //vorticity[dtID.xy] = (velocity[up].y - velocity[down].y) / (2.0 * dx.y)
    //            - (velocity[right].x - velocity[left].x) / (2.0 * dx.x);
    
    vorticity[dtID.xy] = (velocity[right].y - velocity[left].y) / (2.0 / dx.x)
                - (velocity[up].x - velocity[down].x) / (2.0 / dx.y);
}
