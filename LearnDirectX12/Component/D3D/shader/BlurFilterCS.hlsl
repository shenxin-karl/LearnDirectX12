
cbuffer BlurParame : register(b0){
    int   gBlurCount;
    float gBlurWieghts[11];
};


#define N 256
#define kMaxBlurCount 5
#define kExtraCacheCount (kMaxBlurCount * 2)

Texture2D gInput           : register(t0);
RWTexture2D<float4> gOuput : register(u0);
groupshared float gCache[N + kExtraCacheCount];

[numthreads(N, 1, 1)]
void HorizonBlurCS(int3 groupThreadID : SV_GroupThreadID, 
                   int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.x < kMaxBlurCount) {
        int x = max(dispatchThreadID.x - kMaxBlurCount, 0);
        gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x > N - kMaxBlurCount) {
        int x = min(dispatchThreadID.x + kMaxBlurCount, gInput.Length.x-1);
        gCache[groupThreadID.x + kExtraCacheCount] = gInput[int2(x, dispatchThreadID.y)];
    }
    gCache[groupThreadID.x + kMaxBlurCount] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int blurIndex = -gBlurCount; blurIndex <= +gBlurCount; ++blurIndex) {
        int x = dispatchThreadID.x + kMaxBlurCount;
        int index = blurIndex + kMaxBlurCount;
        blurColor += gCache[x + blurIndex] * gBlurWieghts[index];
    }
    //gOuput[dispatchThreadID.xy] = blurColor;
    gOuput[dispatchThreadID.xy] = float4(1, 0, 0, 1);
}

[numthreads(1, N, 1)]
void VerticalBlurCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.y < kMaxBlurCount) {
        int y = max(dispatchThreadID.y - kMaxBlurCount, 0);
        gCache[y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y > (N - kMaxBlurCount)) {
        int y = min(dispatchThreadID.y + kMaxBlurCount, gInput.Length.y-1);
        gCache[groupThreadID.y + kMaxBlurCount] = gInput[int2(dispatchThreadID.x, y)];
    }
    gCache[groupThreadID.y + kExtraCacheCount] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];

    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0.0, 0.0, 0.0, 0.0);
    for (int blurIndex = -gBlurCount; blurIndex <= +gBlurCount; ++blurIndex) {
        int y = dispatchThreadID.y + kMaxBlurCount;
        int index = blurIndex + kMaxBlurCount;
        blurColor += gCache[y + blurIndex] * gBlurWieghts[index];
    }
    //gOuput[dispatchThreadID.xy] = blurColor;
    gOuput[dispatchThreadID.xy] = float4(1, 0, 0, 1);
    
}