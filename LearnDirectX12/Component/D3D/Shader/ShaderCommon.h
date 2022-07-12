#pragma once
#include "Math/MathStd.hpp"
#include "D3D/Tool/D3Dx12.h"
#include <array>

namespace d3d {

using namespace Math;

struct LightData {
	float3  strength;		// ����ǿ��
	float   falloffStart;	// ���Դ/�۹��˥����ʼ����
	float3  direction;		// ����ⷽ��
	float   falloffEnd;		// ���Դ/�۹��˥����������
	float3  position;		// ���Դλ��
	float   spotPower;		// �۹�� pow ָ��
public:
	void initAsDirectionLight(float3 direction, float3 strength);
	void initAsPointLight(float3 position, float3 strength, float falloffStart, float falloffEnd);
	void initAsSpotLight(float3 position, 
		float3 direction, 
		float3 strength, 
		float falloffStart, 
		float falloffEnd, 
		float spotPower
	);
};

struct MaterialData {
	static MaterialData defaultMaterialData;
	float4 diffuseAlbedo;   // ������
	float  roughness;       // �ֲڶ�
	float  metallic;        // ������
	float  padding0 = 0.f;   // ���0
	float  padding1 = 0.f;   // ���1
};

struct CBPassType {
	float4x4 view;			
	float4x4 invView;		
	float4x4 proj;
	float4x4 invProj;
	float4x4 viewProj;
	float4x4 invViewProj;
	float3	 eyePos;	
	float	 cbPerPassPad0 = 0.f;			// padding
	float2	 renderTargetSize;
	float2	 invRenderTargetSize;
	float	 nearZ;
	float	 farZ;
	float	 totalTime;
	float	 deltaTime;
	float4   fogColor;						
	float    fogStart;
	float    fogEnd;
	float2   cbPerPassPad1;
	float4x4 lightSpaceMatrix;
};

constexpr inline std::size_t kMaxLightCount = 16;
struct CBLightType {
	int       directLightCount = 0;
	int       pointLightCount  = 0;
	int       spotLightCount   = 0;
	int       objectPad0       = 0;
	float4	  ambientLight     = float4(0.f);
	LightData lights[kMaxLightCount];
};

CD3DX12_STATIC_SAMPLER_DESC getPointWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getPointClampStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getLinearWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getLinearClampStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getAnisotropicWrapStaticSampler(UINT shaderRegister);
CD3DX12_STATIC_SAMPLER_DESC getAnisotropicClampStaticSampler(UINT shaderRegister);
const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers();

}