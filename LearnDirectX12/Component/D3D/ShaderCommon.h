#pragma once
#include "Math/MathHelper.h"
#include "D3Dx12.h"
#include <array>

namespace d3dutil {

using namespace Math;

struct Light {
	float3  strength;		// 辐射强度
	float   falloffStart;	// 点光源/聚光灯衰减开始距离
	float3  direction;		// 方向光方向
	float   falloffEnd;		// 点光源/聚光灯衰减结束距离
	float3  position;		// 点光源位置
	float   spotPower;		// 聚光灯 pow 指数
public:
	void initAsDirectionLight(float3 direction, float3 strength);
	void initAsPointLight(float3 position, float3 strength, float fallofStart, float fallofEnd);
	void initAsSpotLight(float3 position, 
		float3 direction, 
		float3 strength, 
		float fallofStart, 
		float fallofEnd, 
		float spotPower
	);
};

struct Material {
	float4 diffuseAlbedo;   // 反照率
	float  roughness;       // 粗糙度
	float  metallic;        // 金属度
	float  pading0 = 0.f;   // 填充0
	float  pading1 = 0.f;   // 填充1
};

struct PassCBType {
	float4x4 view;			
	float4x4 invView;		
	float4x4 proj;
	float4x4 invProj;
	float4x4 viewProj;
	float4x4 invViewProj;
	float3	 eyePos;	
	float	 objectPad0 = 0.f;			// padding
	float2	 renderTargetSize;
	float2	 invRenderTargetSize;
	float	 nearZ;
	float	 farZ;
	float	 totalTime;
	float	 deltaTime;
};

constexpr std::size_t kMaxLightCount = 16;
struct LightCBType {
	int      directLightCount;
	int      pointLightCount;
	int      spotLightCount;
	int      objectPad0;
	float4	 ambientLight;
	Light    lights[kMaxLightCount];
};


const CD3DX12_STATIC_SAMPLER_DESC &getPointWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getPointClampStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getLinearWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getLinearClampStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicWrapStaticSampler();
const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicClampStaticSampler();
const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers();

}