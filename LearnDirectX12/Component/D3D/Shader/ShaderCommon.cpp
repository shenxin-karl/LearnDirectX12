#include "ShaderCommon.h"

namespace d3d {
using namespace Math;

MaterialData MaterialData::defaultMaterialData{
	float4(1.f), 1.f, 1.f
};


void LightData::initAsDirectionLight(float3 direction, float3 strength) {
	this->direction = normalize(Vector3(direction)).xyz;
	this->strength = strength;
	this->position = float3(0);
	this->falloffStart = 0.f;
	this->falloffEnd = 0.f;
	this->spotPower = 0.f;
}

void LightData::initAsPointLight(float3 position, float3 strength, float falloffStart, float falloffEnd) {
	this->direction = float3(0);
	this->strength = strength;
	this->position = position;
	this->falloffStart = falloffStart;
	this->falloffEnd = falloffEnd;
	this->spotPower = 0.f;
}

void LightData::initAsSpotLight(float3 position, 
	float3 direction, 
	float3 strength, 
	float falloffStart, 
	float falloffEnd, 
	float spotPower) 
{
	this->direction = normalize(-Vector3(direction)).xyz;
	this->strength = strength;
	this->position = position;
	this->falloffStart = falloffStart;
	this->falloffEnd = falloffEnd;
	this->spotPower = spotPower;
}

CD3DX12_STATIC_SAMPLER_DESC getPointWrapStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getPointClampStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getLinearWrapStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getLinearClampStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getAnisotropicWrapStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getAnisotropicClampStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getLinearShadowCompareStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
	);
}

CD3DX12_STATIC_SAMPLER_DESC getPointShadowCompareStaticSampler(UINT shaderRegister) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderRegister, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
	);
}

const std::array<CD3DX12_STATIC_SAMPLER_DESC, 8> &getStaticSamplers() {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 8> samplers = {
		getPointWrapStaticSampler(0),
		getPointClampStaticSampler(1),
		getLinearWrapStaticSampler(2),
		getLinearClampStaticSampler(3),
		getAnisotropicWrapStaticSampler(4),
		getAnisotropicClampStaticSampler(5),
		getLinearShadowCompareStaticSampler(6),
		getPointShadowCompareStaticSampler(7)
	};
	return samplers;
}

}