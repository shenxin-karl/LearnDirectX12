#pragma once
#include "Math/MathStd.hpp"
#include "D3D/Tool/D3Dx12.h"
#include <array>

namespace d3d {


struct LightData {
	Math::float3  strength;		// 辐射强度
	float		  falloffStart;	// 点光源/聚光灯衰减开始距离
	Math::float3  direction;		// 方向光方向
	float		  falloffEnd;		// 点光源/聚光灯衰减结束距离
	Math::float3  position;		// 点光源位置
	float		  spotPower;		// 聚光灯 pow 指数
public:
	void initAsDirectionLight(Math::float3 direction, Math::float3 strength);
	void initAsPointLight(Math::float3 position, Math::float3 strength, float falloffStart, float falloffEnd);
	void initAsSpotLight(Math::float3 position,
		Math::float3 direction,
		Math::float3 strength,
		float falloffStart, 
		float falloffEnd, 
		float spotPower
	);
};

struct MaterialData {
	static MaterialData defaultMaterialData;
	Math::float4 diffuseAlbedo;   // 反照率
	float  roughness;       // 粗糙度
	float  metallic;        // 金属度
	float  padding0 = 0.f;   // 填充0
	float  padding1 = 0.f;   // 填充1
};

struct CBPassType {
	Math::float4x4 view;
	Math::float4x4 invView;
	Math::float4x4 proj;
	Math::float4x4 invProj;
	Math::float4x4 viewProj;
	Math::float4x4 invViewProj;
	Math::float3   eyePos;
	float		   cbPerPassPad0 = 0.f;			// padding
	Math::float2   renderTargetSize;
	Math::float2   invRenderTargetSize;
	float		   nearZ;
	float		   farZ;
	float		   totalTime;
	float		   deltaTime;
	Math::float4   fogColor;						
	float          fogStart;
	float          fogEnd;
	Math::float2   cbPerPassPad1;
};

constexpr inline std::size_t kMaxLightCount = 16;
struct CBLightType {
	int          directLightCount = 0;
	int          pointLightCount  = 0;
	int          spotLightCount   = 0;
	int          objectPad0       = 0;
	Math::float4 ambientLight     = Math::float4(0.f);
	LightData    lights[kMaxLightCount];
};

struct CSMSubFrustum {
	float width;
	float height;
	float zNear;
	float zFar;
	Math::float4x4 worldToLightMatrix;
	Math::float3   center;
	float		   lightPlane;
};

constexpr inline std::size_t kMaxShadowCascaded = 4;
struct CBShadowType {
	Math::float3  lightDir;
	float		  lightSize;
	CSMSubFrustum subFrustum[kMaxShadowCascaded];
};

CD3DX12_STATIC_SAMPLER_DESC getPointWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getPointClampStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getLinearWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getLinearClampStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getAnisotropicWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getAnisotropicClampStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getLinearShadowCompareStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getPointShadowCompareStaticSampler(UINT shaderRegister);
const std::array<CD3DX12_STATIC_SAMPLER_DESC, 8> &getStaticSamplers();

}