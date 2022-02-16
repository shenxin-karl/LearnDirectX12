#pragma once
#include "Math/MathHelper.h"
#include "D3Dx12.h"
#include <array>

namespace d3dutil {

using namespace Math;

struct Light {
	float3  strength;		// ����ǿ��
	float   falloffStart;	// ���Դ/�۹��˥����ʼ����
	float3  direction;		// ����ⷽ��
	float   falloffEnd;		// ���Դ/�۹��˥����������
	float3  position;		// ���Դλ��
	float   spotPower;		// �۹�� pow ָ��
};

struct Material {
	float3 diffuseAlbedo;	// ������
	float  roughness;		// �ֲڶ�
	float3 fresnelR0;		// ������ϵ��
	float  metallic;		// ������
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