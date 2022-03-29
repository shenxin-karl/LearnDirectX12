#include "ShaderCommon.h"

namespace d3d {

void Light::initAsDirectionLight(float3 direction, float3 strength) {
	this->direction = normalize(direction);
	this->strength = strength;
	this->position = float3(0);
	this->falloffStart = 0.f;
	this->falloffEnd = 0.f;
	this->spotPower = 0.f;
}

void Light::initAsPointLight(float3 position, float3 strength, float fallofStart, float fallofEnd) {
	this->direction = float3(0);
	this->strength = strength;
	this->position = position;
	this->falloffStart = fallofStart;
	this->falloffEnd = fallofEnd;
	this->spotPower = 0.f;
}

void Light::initAsSpotLight(float3 position, 
	float3 direction, 
	float3 strength, 
	float fallofStart, 
	float fallofEnd, 
	float spotPower) 
{
	this->direction = normalize(direction);
	this->strength = strength;
	this->position = position;
	this->falloffStart = fallofStart;
	this->falloffEnd = fallofEnd;
	this->spotPower = spotPower;
}

CD3DX12_STATIC_SAMPLER_DESC getPointWrapStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getPointClampStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getLinearWrapStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getLinearClampStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getAnisotropicWrapStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
}

CD3DX12_STATIC_SAMPLER_DESC getAnisotropicClampStaticSampler(UINT shaderReginster) {
	return CD3DX12_STATIC_SAMPLER_DESC(
		shaderReginster,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
}

const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers() {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> samplers = {
		getPointWrapStaticSampler(0),
		getPointClampStaticSampler(1),
		getLinearWrapStaticSampler(2),
		getLinearClampStaticSampler(3),
		getAnisotropicWrapStaticSampler(4),
		getAnisotropicClampStaticSampler(5),
	};
	return samplers;
}

}