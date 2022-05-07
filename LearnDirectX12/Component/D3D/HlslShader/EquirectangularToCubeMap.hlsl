#ifndef N
    #define N 32
#endif

cbuffer CBSettings : register(b0) {
	float gWidth;
    float gHeight;
};

Texture2D<float4>   gEquirectangularMap : register(t0);
RWTexture2D<float3> gCubeMapFace[6]     : register(u0);     // Right
SamplerState        gSamLinearClamp     : register(s0);

struct CSIn {
	uint3 	GroupID 		  : SV_GroupID;				// 线程组 ID
    uint3   GroupThreadID	  : SV_GroupThreadID;		// 组内线程 ID
    uint3   DispatchThreadID  : SV_DispatchThreadID;	// 线程 ID
    uint  	GroupIndex        : SV_GroupIndex;			// 线程组内索引
};

static float3x3 TBNArray[6]  = {
	float3x3(float3(+1.0, 0.0, 0.0), float3(0.0, +1.0, 0.0), float3(0.0, 0.0, +1.0)),
	float3x3(float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0), float3(0.0, 0.0, -1.0)),
    float3x3(float3(0.0, +1.0, 0.0), float3(0.0, 0.0, +1.0), float3(+1.0, 0.0, 0.0)),
    float3x3(float3(0.0, -1.0, 0.0), float3(0.0, 0.0, -1.0), float3(-1.0, 0.0, 0.0)),
    float3x3(float3(0.0, 0.0, +1.0), float3(+1.0, 0.0, 0.0), float3(0.0, +1.0, 0.0)),
    float3x3(float3(0.0, 0.0, -1.0), float3(-1.0, 0.0, 0.0), float3(0.0, -1.0, 0.0)),
};

static float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v) {
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

[numthreads(32, 32, 1)]
void CSMain(CSIn csin) {
	float3 cubeLeftTopCorner = float3(-0.5, +0.5, +0.5);
    float offsetX = 1.0 - (csin.DispatchThreadID.x / gWidth);
    float offsetY = (csin.DispatchThreadID.y / gHeight);
    float3 direction = cubeLeftTopCorner + float3(offsetX, offsetY, 0.0);
    //gCubeMapFace[0][csin.DispatchThreadID.xy] = float3(offsetY, offsetY, offsetY);
    //gCubeMapFace[0][csin.DispatchThreadID.xy] = direction;


#if 1
    [unroll(6)]
    for (int i = 0; i < 6; ++i) {
	    float3 v = mul(direction, TBNArray[i]);
        float2 texcoord = SampleSphericalMap(v);
        gCubeMapFace[i][csin.DispatchThreadID.xy] = normalize(float3(v));//gEquirectangularMap.SampleLevel(gSamLinearClamp, texcoord, 0);
    }
#endif
}