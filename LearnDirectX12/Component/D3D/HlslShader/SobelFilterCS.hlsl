
Texture2D           gInput  : register(t0);

#ifdef APPLY_MODE
RWTexture2D<float4> gOutput : register(u0);
#endif
#ifdef PRODUCE_MODE
RWTexture2D<float> gOutput : register(u0);
#endif

float CalcLuminance(float3 color) {
    return dot(color, float3(0.299, 0.587, 0.114));
}

float CalcSobelMag(int3 dispatchThreadID, inout float4 c[3][3]) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int2 xy = dispatchThreadID.xy + int2(i, j);
            c[i+1][j+1] = gInput[xy];
        }
    }
    
   /*
    *   (0, 0)  (1, 0)  (2, 0)
    *   (0, 1)  (1, 1)  (2, 1)
    *   (0, 2)  (1, 2)  (2, 2)
    * 
    * GX =  -1  +0  +1
    *       -2  +0  +2
    *       -1  +0  +1
    *
    * GY =  +1  +2  +1
    *       +0  +0  +0
    *       -1  -2  -1
    */
  
    float4 GX = (-1.0 * c[0][0]) + (+1.0 * c[2][0]) +
                (-2.0 * c[0][1]) + (+2.0 * c[2][1]) +
                (-1.0 * c[0][2]) + (+1.0 * c[2][2]) ;
                
    float4 GY = (+1.0 * c[0][0]) + (+2.0 * c[1][0]) + (+1.0 * c[2][0]) +
                (-1.0 * c[0][2]) + (-2.0 * c[1][2]) + (-1.0 * c[2][2]) ;
                
    float4 G  = sqrt(GX*GX + GY*GY);
    float mag = saturate(CalcLuminance(G.rgb));
    return mag;
}

[numthreads(16, 16, 1)]
void SobelProduce(int3 dispatchThreadID : SV_DispatchThreadID) {
    float4 c[3][3];
    float mag = CalcSobelMag(dispatchThreadID, c);
    gOutput[dispatchThreadID.xy] = 1.0 - mag;
}

[numthreads(16, 16, 1)]
void SobelApply(int3 dispatchThreadID : SV_DispatchThreadID) {
    float4 c[3][3];
    float mag = CalcSobelMag(dispatchThreadID, c);
    gOutput[dispatchThreadID.xy] = (1.0 - mag) * c[1][1];
}