
cbuffer BlurParame : register(b0){
    int   gBlurCount;
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
#define kMaxBlurCount 5
#define kExtraCacheCount (kMaxBlurCount * 2)

Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);
groupshared float gCache[N + kExtraCacheCount];

[numthreads(N, 1, 1)]
void HorizonBlurCS(int3 groupThreadID : SV_GroupThreadID, 
                   int3 dispatchThreadID : SV_DispatchThreadID)
{
#if 0
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
    float blurWeights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    for (int blurIndex = -gBlurCount; blurIndex <= +gBlurCount; ++blurIndex) {
        int x = groupThreadID.x + kMaxBlurCount;
        int index = blurIndex + kMaxBlurCount;
        blurColor += gCache[x + blurIndex] * blurWeights[index];
    }
    gOuput[dispatchThreadID.xy] = blurColor;
#endif
	// Put in an array for each indexing.
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
    if (groupThreadID.x < gBlurCount)
    {
		// Clamp out of bound samples that occur at image borders.
        int x = max(dispatchThreadID.x - gBlurCount, 0);
        gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    if (groupThreadID.x >= N - gBlurCount)
    {
		// Clamp out of bound samples that occur at image borders.
        int x = min(dispatchThreadID.x + gBlurCount, gInput.Length.x - 1);
        gCache[groupThreadID.x + 2 * gBlurCount] = gInput[int2(x, dispatchThreadID.y)];
    }

	// Clamp out of bound samples that occur at image borders.
    gCache[groupThreadID.x + gBlurCount] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
    GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurCount; i <= gBlurCount; ++i)
    {
        int k = groupThreadID.x + gBlurCount + i;
		
        blurColor += weights[i + gBlurCount] * gCache[k];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void VerticalBlurCS(int3 groupThreadID : SV_GroupThreadID,
                    int3 dispatchThreadID : SV_DispatchThreadID)
{
#if 0
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
    float blurWeights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };
    for (int blurIndex = -gBlurCount; blurIndex <= +gBlurCount; ++blurIndex) {
        int y = groupThreadID.y + kMaxBlurCount;
        int index = blurIndex + kMaxBlurCount;
        blurColor += gCache[y + blurIndex] * blurWeights[index];
    }
    gOuput[dispatchThreadID.xy] = blurColor;
#endif

	// Put in an array for each indexing.
    float weights[11] = { w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//
	
	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
    if (groupThreadID.y < gBlurCount)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = max(dispatchThreadID.y - gBlurCount, 0);
        gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    if (groupThreadID.y >= N - gBlurCount)
    {
		// Clamp out of bound samples that occur at image borders.
        int y = min(dispatchThreadID.y + gBlurCount, gInput.Length.y - 1);
        gCache[groupThreadID.y + 2 * gBlurCount] = gInput[int2(dispatchThreadID.x, y)];
    }
	
	// Clamp out of bound samples that occur at image borders.
    gCache[groupThreadID.y + gBlurCount] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];


	// Wait for all threads to finish.
    GroupMemoryBarrierWithGroupSync();
	
	//
	// Now blur each pixel.
	//

    float4 blurColor = float4(0, 0, 0, 0);
	
    for (int i = -gBlurCount; i <= gBlurCount; ++i)
    {
        int k = groupThreadID.y + gBlurCount + i;
		
        blurColor += weights[i + gBlurCount] * gCache[k];
    }
	
    gOutput[dispatchThreadID.xy] = blurColor;
}