#include "IBL.h"
#include "D3D/Shader/D3DShaderResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stb/stb_image.h>
#include <random>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Pipeline/PipelineStd.h>
#include <dx12lib/Texture/TextureStd.h>
#include <dx12lib/Buffer/BufferStd.h>

#include "D3D/d3dutil.h"
#include "D3D/Shader/ShaderCommon.h"
#include "dx12lib/Buffer/ConsumeStructuredBuffer.h"
#include "Geometry/GeometryGenerator.h"

namespace d3d {

IBL::IBL(dx12lib::DirectContextProxy pComputeCtx, const std::string &fileName) {
	std::memset(&_irradianceMapSH3, 0, sizeof(_irradianceMapSH3));

	std::wstring wcharFileName = std::to_wstring(fileName);
	std::wstring_view suffix = L".hdr";
	if (auto iter = wcharFileName.find(suffix); iter != std::wstring::npos)
		wcharFileName.replace(iter, iter + suffix.length(), L".dds");
	else {
		assert(false);
	}
	
	auto pPannoEnvMap = pComputeCtx->createDDSTexture2DFromFile(wcharFileName);
	buildPanoToCubeMapPSO(pComputeCtx->getDevice());
	buildEnvMap(pComputeCtx, pPannoEnvMap);

	// build irradiance map spherical harmonics
	buildConvolutionIrradiancePSO(pComputeCtx->getDevice());
	buildConvolutionIrradianceMap(pComputeCtx, pPannoEnvMap);

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pComputeCtx->createDDSTexture2DFromMemory(brdfLutFile.begin(), brdfLutFile.size());

	pComputeCtx->trackResource(std::move(pPannoEnvMap));
}

std::shared_ptr<dx12lib::UnorderedAccessCube> IBL::getEnvMap() const {
	return _pEnvMap;
}

const SH3 &IBL::getIrradianceMapSH3() const {
	return _irradianceMapSH3;
}

void IBL::buildPanoToCubeMapPSO(std::weak_ptr<dx12lib::Device> pDevice) {
	auto pSharedDevice = pDevice.lock();

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	samplers.push_back(d3d::getLinearClampStaticSampler(0));
	dx12lib::RootSignatureDescHelper desc(samplers);
	desc.resize(3);
	desc[CB_Settings].InitAsConstants(3, 0, 0);
	desc[SR_EnvMap].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,  1, 0);
	desc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	auto pRootSignature = pSharedDevice->createRootSignature(desc);

	cmrc::file shaderContent = d3d::getD3DResource("HlslShader/PanoToCubeMapCS.hlsl");
	_pPanoToCubeMapPSO = pSharedDevice->createComputePSO("PanoToCubeMapPSO");
	_pPanoToCubeMapPSO->setRootSignature(pRootSignature);
	_pPanoToCubeMapPSO->setComputeShader(d3d::compileShader(
		shaderContent.begin(), 
		shaderContent.size(), 
		nullptr, 
		"CS", 
		"cs_5_0")
	);
	_pPanoToCubeMapPSO->finalize();
}

void IBL::buildConvolutionIrradiancePSO(std::weak_ptr<dx12lib::Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	dx12lib::RootSignatureDescHelper desc(d3d::getLinearClampStaticSampler(0));
	desc.resize(3);
	desc[CB_Settings].InitAsConstants(3, 0, 0);
	desc[SR_EnvMap].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	desc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	auto pRootSignature = pSharedDevice->createRootSignature(desc);
	cmrc::file shaderContent = d3d::getD3DResource("HlslShader/ConvolutionIrradianceMapCS.hlsl");
	_pConvolutionIrradiancePSO = pSharedDevice->createComputePSO("ConvolutionIrradiancePSO");
	_pConvolutionIrradiancePSO->setRootSignature(pRootSignature);
	_pConvolutionIrradiancePSO->setComputeShader(d3d::compileShader(
		shaderContent.begin(),
		shaderContent.size(),
		nullptr,
		"CS",
		"cs_5_0")
	);
	_pConvolutionIrradiancePSO->finalize();
}

void IBL::buildEnvMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap) {
	size_t width = pPannoEnvMap->getWidth();
	size_t height = pPannoEnvMap->getHeight();
	size_t size = std::max(width, height) / 3;

	float fWidth = static_cast<float>(width);
	float fHeight = static_cast<float>(height);
	float fSize = static_cast<float>(size);
	_pEnvMap = pComputeCtx->createUnorderedAccessCube(size, size, nullptr, DXGI_FORMAT_R16G16B16A16_FLOAT);
	pComputeCtx->setComputePSO(_pPanoToCubeMapPSO);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 1);
	pComputeCtx->setShaderResourceView(pPannoEnvMap->getSRV(), SR_EnvMap);
	pComputeCtx->setUnorderedAccessView(_pEnvMap->get2DArrayUAV(), UA_Output);
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	pComputeCtx->dispatch(groupX, groupY, 6);

	auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	pComputeCtx->transitionBarrier(_pEnvMap, state);
}

struct SH3Coeff {
	float3 m[9];
};

void IBL::buildConvolutionIrradianceMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap) {
	size_t width = pPannoEnvMap->getWidth();
	size_t height = pPannoEnvMap->getHeight();
	size_t size = std::max(width, height) / 3;
	float fSize = static_cast<float>(size);
	float fStep = 0.075f;

	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / 8));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / 8));
	size_t outputSize = (groupY * groupX) * 6;
	auto pCSOutput = pComputeCtx->createUAStructuredBuffer(nullptr, outputSize, sizeof(SH3Coeff));
	auto pReadBack = pComputeCtx->createReadBackBuffer(outputSize, sizeof(SH3Coeff));

	pComputeCtx->setComputePSO(_pConvolutionIrradiancePSO);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 1);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fStep, 2);
	pComputeCtx->setShaderResourceView(_pEnvMap->getSRV(), SR_EnvMap);
	pComputeCtx->setUnorderedAccessView(pCSOutput->getUAV(), UA_Output);
	pComputeCtx->dispatch(groupX, groupY, 6);

	pComputeCtx->copyResource(pReadBack, pCSOutput);
	pComputeCtx->readBack(pReadBack);
	pReadBack->setCompletedCallback([=](dx12lib::IReadBackBuffer *pResource) {
		assert(pResource);
		const dx12lib::ReadBackBuffer *pReadBackResource = static_cast<dx12lib::ReadBackBuffer *>(pResource);
		std::span<const SH3Coeff> bufferVisitor = pReadBackResource->visit<SH3Coeff>();

		Vector3 coeffs[9];
		std::fill(std::begin(coeffs), std::end(coeffs), Vector3(0));
		for (const auto &shCoeffs : bufferVisitor) {
			for (size_t i = 0; i < 9; ++i)
				coeffs[i] += Vector3(shCoeffs.m[i]);
		}

		float normalizingFactor = ((4.f * DX::XM_PI) / (size * size * 6));
		for (size_t j = 0; j < 9; ++j)
			_irradianceMapSH3._m[j] = float4(coeffs[j] * normalizingFactor);
	});

	pComputeCtx->trackResource(std::move(pCSOutput));
	pComputeCtx->trackResource(std::move(pReadBack));
}

}

