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

	auto pRootSignature = pSharedDevice->createRootSignature(2, 1);
	pRootSignature->initStaticSampler(0, getLinearClampStaticSampler(0));
	pRootSignature->at(1).initAsConstants(dx12lib::RegisterSlot::CBV0, 2);
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::SRV0, 1 },
		{ dx12lib::RegisterSlot::UAV0, 1 },
	});
	pRootSignature->finalize();

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
	auto pRootSignature = pSharedDevice->createRootSignature(2, 1);
	pRootSignature->initStaticSampler(0, getLinearWrapStaticSampler(0));
	pRootSignature->at(0).initAsConstants(dx12lib::RegisterSlot::CBV0, 3);
	pRootSignature->at(1).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::SRV0, 1 },
		{ dx12lib::RegisterSlot::UAV0, 2 },
	});
	pRootSignature->finalize();

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
	_pEnvMap = pComputeCtx->createUnorderedAccessCube(size, size, 1, nullptr, DXGI_FORMAT_R16G16B16A16_FLOAT);
	pComputeCtx->setComputePSO(_pPanoToCubeMapPSO);
	pComputeCtx->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &fSize, 1);
	pComputeCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, pPannoEnvMap->getSRV());
	pComputeCtx->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, _pEnvMap->get2DArrayUAV());
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	pComputeCtx->dispatch(groupX, groupY, 6);

	auto state = D3D12_RESOURCE_STATE_GENERIC_READ;
	pComputeCtx->transitionBarrier(_pEnvMap, state);
}

struct SH3Coeff {
	float3 m[9];
};

void IBL::buildConvolutionIrradianceMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap) {
	size_t width = pPannoEnvMap->getWidth();
	size_t height = pPannoEnvMap->getHeight();
	size_t size = 128;
	float fSize = static_cast<float>(size);
	float fStep = 0.075f;

	_pTest = pComputeCtx->createUnorderedAccessCube(size, size, 1);
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / 8));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / 8));
	size_t outputSize = (groupY * groupX) * 6;
	auto pCSOutput = pComputeCtx->createUAStructuredBuffer(nullptr, outputSize, sizeof(SH3Coeff));
	auto pReadBack = pComputeCtx->createReadBackBuffer(outputSize, sizeof(SH3Coeff));

	pComputeCtx->setComputePSO(_pConvolutionIrradiancePSO);
	pComputeCtx->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &fSize, 1);
	pComputeCtx->setCompute32BitConstants(dx12lib::RegisterSlot::CBV0, 1, &fStep, 2);
	pComputeCtx->setShaderResourceView(dx12lib::RegisterSlot::SRV0, _pEnvMap->getSRV());
	pComputeCtx->setUnorderedAccessView(dx12lib::RegisterSlot::UAV0, pCSOutput->getUAV());
	pComputeCtx->setUnorderedAccessView(dx12lib::RegisterSlot::UAV1, _pTest->get2DArrayUAV());
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

		float normalizingFactor = 1.f;//((4.f * DX::XM_PI) / (size * size * 6));
		for (size_t j = 0; j < 9; ++j)
			_irradianceMapSH3._m[j] = float4(coeffs[j] * normalizingFactor);
	});

	pComputeCtx->transitionBarrier(_pTest, D3D12_RESOURCE_STATE_GENERIC_READ);

	pComputeCtx->trackResource(std::move(pCSOutput));
	pComputeCtx->trackResource(std::move(pReadBack));
}

void IBL::buildPerFilterEnvPSO(std::weak_ptr<dx12lib::Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	auto pRootSignature = pSharedDevice->createRootSignature(1, 1);
	pRootSignature->initStaticSampler(0, getLinearWrapStaticSampler(0));
	pRootSignature->at(0).initAsDescriptorTable({
		{ dx12lib::RegisterSlot::CBV0, 1 },
		{ dx12lib::RegisterSlot::SRV0, 1 },
		{ dx12lib::RegisterSlot::UAV0, 5 },
	});
	pRootSignature->finalize();

	cmrc::file shaderContents = getD3DResource("HlslShader/PerFilterEnvMapCS.hlsl");
	_pPerFilterEnvPSO = pSharedDevice->createComputePSO("PerFilterEnvPSO");
	_pPerFilterEnvPSO->setRootSignature(pRootSignature);
	_pPerFilterEnvPSO->setComputeShader(compileShader(
		shaderContents.begin(),
		shaderContents.size(),
		nullptr,
		"CS",
		"cs_5_0"
	));
	_pPerFilterEnvPSO->finalize();
}

void IBL::buildPerFilterEnvMap(dx12lib::ComputeContextProxy pComputeCtx) {
	//_pPerFilterEnvMap = pComputeCtx->createUnorderedAccessCube(256, 256, )
}

}

