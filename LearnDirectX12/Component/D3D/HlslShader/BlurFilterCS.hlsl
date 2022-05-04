
cbuffer BlurParame : register(b0){
    int   gBlurRaidus;
    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
};


#define N 256
#define kMaxBlurRadius 5
#define kExtraCacheCount (kMaxBlurRadius * 2)

Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);
groupshared float4  gCache[N + kExtraCacheCount];

[numthreads(N, 1, 1)]
void HorizonBlurCS(int3 groupThreadID : SV_GroupThreadID, 
                   int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.x < kMaxBlurRadius) {
        int x = max(dispatchThreadID.x - kMaxBlurRadius, 0);
        gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x >= (N - kMaxBlurRadius)) {
        int x = min(dispatchThreadID.x + kMaxBlurRadius, gInput.Length.x-1);
        gCache[groupThreadID.x + kExtraCacheCount] = gInput[int2(x, dispatchThreadID.y)];
    }
    gCache[groupThreadID.x + kMaxBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];
    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0, 0, 0, 0);
    float blurWeights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    for (int blurIndex = -gBlurRaidus; blurIndex <= +gBlurRaidus; ++blurIndex) {
        int x = groupThreadID.x + kMaxBlurRadius + blurIndex;
        int index = blurIndex + kMaxBlurRadius;
        blurColor += gCache[x] * blurWeights[index];
    }
    gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VerticalBlurCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.y < kMaxBlurRadius) {
        int y = max(dispatchThreadID.y - kMaxBlurRadius, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= (N - kMaxBlurRadius)) {
        int y = min(dispatchThreadID.y + kMaxBlurRadius, gInput.Length.y-1);
        gCache[groupThreadID.y + kExtraCacheCount] = gInput[int2(dispatchThreadID.x, y)];
    }
    gCache[groupThreadID.y + kMaxBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy-1)];
    GroupMemoryBarrierWithGroupSync();

    float4 blurColor = float4(0, 0, 0, 0);
    float blurWeights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    for (int blurIndex = -gBlurRaidus; blurIndex <= +gBlurRaidus; ++blurIndex) {
        int y = groupThreadID.y + kMaxBlurRadius + blurIndex;
        int index = blurIndex + kMaxBlurRadius;
        blurColor += gCache[y] * blurWeights[index];
    }
    gOutput[dispatchThreadID.xy] = blurColor;
}