
cbuffer FXAASetting : register(b0) {
    float gMinThreshold;            // 最小亮度差阈值
    float gThreshold;               // 最大亮度差阈值
    float gConsoleTangentSacle;     // 边缘切线上的缩放一般取值 0.5
    float gSharpness;               // 当切向量垂直时上的缩放
};

Texture2D           gInput  : register(t0);
RWTexture2D<float4> gOutput : register(u0);

float CalcLuminance(float3 color) {
    return dot(color, float3(0.299, 0.587, 0.114));
}

SamplerState gSamLinearClamp : register(s0);

[numthreads(16, 16, 1)]
void FXAAConsole(int3 dispatchThreadID : SV_DispatchThreadID) {
    float s = float(dispatchThreadID.x) / float(gInput.Length.x-1);
    float t = float(dispatchThreadID.y) / float(gInput.Length.y-1);
    float invWidth  = 1.0 / (gInput.Length.x-1);
    float invHeight = 1.0 / (gInput.Length.y-1);
    float invHalfWidth  = invWidth * 0.5;
    float invHalfHeight = invHeight * 0.5;
    float4 pixelColor = gInput.SampleLevel(gSamLinearClamp, float2(s, t), 0.0);
    float M  = CalcLuminance(pixelColor.rgb);
    float NW = CalcLuminance(gInput.SampleLevel(gSamLinearClamp, float2(s-invHalfWidth, t+invHalfHeight), 0.0).rgb);
    float NE = CalcLuminance(gInput.SampleLevel(gSamLinearClamp, float2(s+invHalfWidth, t+invHalfHeight), 0.0).rgb);
    float SW = CalcLuminance(gInput.SampleLevel(gSamLinearClamp, float2(s-invHalfWidth, t-invHalfHeight), 0.0).rgb);
    float SE = CalcLuminance(gInput.SampleLevel(gSamLinearClamp, float2(s+invHalfWidth, t-invHalfHeight), 0.0).rgb);
    float maxLuma = max(max(NW, NE), max(SW, SE));
    float minLuma = min(min(NW, NE), min(SW, SE));
    float contrast = max(maxLuma, M) - min(minLuma, M);
    if (contrast < max(gMinThreshold, maxLuma*gThreshold)) {
        gOutput[dispatchThreadID.xy] = pixelColor;
        return;
    }
    
    float2 N = float2((NE+SE) - (NW+SW), (NW+NE) - (SW+SE));
    float2 tangent = float2(-N.y, N.x);
    float2 dir = normalize(tangent);
    float2 dir1 = dir * float2(invWidth, invHeight) * gConsoleTangentSacle;
    
    float4 S0 = gInput.SampleLevel(gSamLinearClamp, float2(s, t) + dir1, 0.0);
    float4 L0 = gInput.SampleLevel(gSamLinearClamp, float2(s, t) - dir1, 0.0);
    float4 result1 = (S0 + L0) * 0.5;
    
    float minDirAbsAxis = min(abs(dir.x), abs(dir.y)) * gSharpness;
    float2 dir2 = clamp(dir / minDirAbsAxis, -2, +2) * 2 * float2(invWidth, invHeight);
    float4 S1 = gInput.SampleLevel(gSamLinearClamp, float2(s, t) + dir2, 0.0);
    float4 L1 = gInput.SampleLevel(gSamLinearClamp, float2(s, t) - dir2, 0.0);
    float4 result2 = result1 * 0.5 + (S1 + L1) * 0.25;
    if (CalcLuminance(result2.rgb) >= minLuma && CalcLuminance(result2.rgb) <= maxLuma)
        result1 = result2;
        
    gOutput[dispatchThreadID.xy] = result1;
}