#include "ShaderCommand.h"

namespace d3dUtil {

	const CD3DX12_STATIC_SAMPLER_DESC &getPointWrapStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	return sampler;
}

const CD3DX12_STATIC_SAMPLER_DESC &getPointClampStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	return sampler;
}

const CD3DX12_STATIC_SAMPLER_DESC &getLinearWrapStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	return sampler;
}

const CD3DX12_STATIC_SAMPLER_DESC &getLinearClampStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	return sampler;
}

const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicWrapStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP
	);
	return sampler;
}

const CD3DX12_STATIC_SAMPLER_DESC &getAnisotropicClampStaticSampler() {
	static CD3DX12_STATIC_SAMPLER_DESC sampler(
		5,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP
	);
	return sampler;
}

const std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> &getStaticSamplers() {
	static std::array<CD3DX12_STATIC_SAMPLER_DESC, 6> samplers = {
		getPointWrapStaticSampler(),
		getPointClampStaticSampler(),
		getLinearWrapStaticSampler(),
		getLinearClampStaticSampler(),
		getAnisotropicWrapStaticSampler(),
		getAnisotropicClampStaticSampler(),
	};
	return samplers;
}

}