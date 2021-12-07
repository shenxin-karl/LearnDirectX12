#ifndef _PASS_CONSTANT_BUFFER_HLSL_
#define _PASS_CONSTANT_BUFFER_HLSL_

#if !defined(USE_DEF_PASS_REG) || defined(PASS_REGISTER_ID)
	#define PASS_REGISTER_ID b1
#endif

cbuffer PassConstants : register(PASS_REGISTER_ID) {
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
};

#endif