#ifndef _COMMON_H_
#define _COMMON_H_

struct Light {
	float3  strength;
	float   falloffStart;
	float3  direction;
	float   falloffEnd;
	float3  position;
	float   spotPower;
};

struct Material {
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float  roughness;
	float  metallic;
};

struct PassCBType {
	float4x4 view;
	float4x4 invView;
	float4x4 proj;
	float4x4 invProj;
	float4x4 viewProj;
	float4x4 invViewProj;
	float3	 eyePos;
	float	 objectPad0;			// padding
	float2	 renderTargetSize;
	float2	 invRenderTargetSize;
	float	 nearZ;
	float	 farZ;
	float	 totalTime;
	float	 deltaTime;
};

#define kMaxLightCount 16
struct LightCBType {
	int      directLightCount;
	int      pointLightCount;
	int      spotLightCount;
	int      objectPad0;
	float4	 ambientLight;
	Light    lights[kMaxLightCount];
};

#ifndef	DISABLE_DEFAULT_SAMPLER
SamplerState gSamPointWrap        : register(s0);
SamplerState gSamPointClamp       : register(s1);
SamplerState gSamLinearWrap       : register(s2);
SamplerState gSamLinearClamp      : register(s3);
SamplerState gSamAnisotropicWrap  : register(s4);
SamplerState gSamAnisotropicClamp : register(s5);
#endif // !DISABLE_DEFAULT_SAMPLER

#endif // !_COMMON_H_