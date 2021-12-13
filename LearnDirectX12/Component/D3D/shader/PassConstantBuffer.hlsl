#ifndef __PASS_CONSTANT_BUFFER_HLSL__
#define __PASS_CONSTANT_BUFFER_HLSL__

cbuffer PassConstants : register(CB_PASS_TYPE){
	float4x4	gView;
	float4x4	gInvView;
	float4x4	gProj;
	float4x4	gInvProj;
	float4x4	gViewProj;
	float4x4	gInvViewProj;
	float3		gEyePos;
	float		cbPerObjectPad1;		// padding
	float2		gRenderTargetSize;
	float2		gInvRenderTargetSize;
	float		gNearZ;
	float		gFarZ;
	float		gTotalTime;
	float		gDeltaTime;
	float4		gAmbientLight;
	Light		gLights[MAX_LIGHTS];
};

#endif