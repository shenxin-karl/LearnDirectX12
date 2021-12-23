#ifndef __PASS_CONSTANT_BUFFER_HLSL__
#define __PASS_CONSTANT_BUFFER_HLSL__

cbuffer PassConstantsBuffer : register(CB_PASS_TYPE){
	float4x4	gView;
	float4x4	gInvView;
	float4x4	gProj;
	float4x4	gInvProj;
	float4x4	gViewProj;
	float4x4	gInvViewProj;
	float3		gEyePos;
	float		gObjectPad0;		// padding
	float2		gRenderTargetSize;
	float2		gInvRenderTargetSize;
	float		gNearZ;
	float		gFarZ;
	float		gTotalTime;
	float		gDeltaTime;
	float4		gAmbientLight;
	Light		gLights[MAX_LIGHTS];
};

cbuffer CBObjectBuffer : register(CB_OBJECT_TYPE) {
    float4x4    gWorldMat;
    float4x4    gInvWorldMat;
    float4x4    gTextureMat;
    float3x4    gNormalMat;
	int         gObjectID;
	int         gPadding0;
	float2		gPadding1;
};

#endif