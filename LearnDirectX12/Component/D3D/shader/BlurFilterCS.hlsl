
cbuffer BlurParame : register(b0){
    int   gBlurCount;
    float gBlurWieghts[11];
};


#define N 256

Texture2D gInput           : register(t0);
RWTexture2D<float4> gOuput : register(u0);
groupshared float gCache[N];

[numthreads(N, 1, 1)]
void HorizonBlurCS(int3 groupThreadID : SV_GroupThreadID, 
                   int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.x < N)
        gCache[groupThreadID.x] = gInput[dispatchThreadID.xy];
    
    GroupMemoryBarrierWithGroupSync();
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    for (int blurIndex = -gBlurCount; blurIndex <= gBlurCount; ++blurIndex) {
        int x = clamp(blurIndex + dispatchThreadID.x, 0, N);
        int index = x - dispatchThreadID.x + gBlurCount;
        sum += gCache[x] * gBlurWieghts[index];
    }
    gOuput[dispatchThreadID.xy] = sum;
}

[numthreads(1, N, 1)]
void VerticalBlurCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.y < N)
        gCache[groupThreadID.y] = gInput[dispatchThreadID.xy];

    GroupMemoryBarrierWithGroupSync();
    float4 sum = float4(0.0, 0.0, 0.0, 0.0);
    for (int blurIndex = -gBlurCount; blurIndex <= gBlurCount; ++blurIndex) {
        int y = clamp(blurIndex + dispatchThreadID.y, 0, N);
        int index = y - dispatchThreadID.y + gBlurCount;
        sum += gCache[y] * gBlurWieghts[index];
    }
    gOuput[dispatchThreadID.xy] = sum;
}