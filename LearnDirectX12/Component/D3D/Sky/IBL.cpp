#include "IBL.h"
#include "D3D/Shader/D3DShaderResource.h"

#define STB_IMAGE_IMPLEMENTATION
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

	buildConvolutionIrradiancePSO(pComputeCtx->getDevice());
	buildConvolutionIrradianceMap(pComputeCtx, pPannoEnvMap);

	buildIrradianceMapToSHPSO(pComputeCtx->getDevice());
	buildIrradianceMapSH(pComputeCtx);

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pComputeCtx->createDDSTexture2DFromMemory(brdfLutFile.begin(), brdfLutFile.size());

	pComputeCtx->trackResource(std::move(pPannoEnvMap));
}

std::shared_ptr<dx12lib::UnorderedAccessCube> IBL::getEnvMap() const {
	return _pEnvMap;
}

std::shared_ptr<dx12lib::UnorderedAccessCube> IBL::getIrradianceMap() const {
	return _pIrradianceMap;
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

void IBL::buildIrradianceMapToSHPSO(std::weak_ptr<dx12lib::Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	dx12lib::RootSignatureDescHelper rootDesc(d3d::getLinearClampStaticSampler(0));
	rootDesc.resize(3);
	rootDesc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootDesc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	rootDesc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);
	auto pRootSignature = pSharedDevice->createRootSignature(rootDesc);

	cmrc::file shaderContent = d3d::getD3DResource("HlslShader/IrradianceMapSHCS.hlsl");
	_pIrradianceMapToSHPSO = pSharedDevice->createComputePSO("IrradianceMapToSHPSO");
	_pIrradianceMapToSHPSO->setRootSignature(pRootSignature);
	_pIrradianceMapToSHPSO->setComputeShader(d3d::compileShader(
		shaderContent.begin(),
		shaderContent.size(),
		nullptr,
		"CS",
		"cs_5_0"
	));
	_pIrradianceMapToSHPSO->finalize();
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
	size_t size = std::max(width, height) / 6;

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

void IBL::buildConvolutionIrradianceMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap) {
	size_t width = pPannoEnvMap->getWidth();
	size_t height = pPannoEnvMap->getHeight();
	size_t size = std::max(width, height) / 6;
	float fSize = static_cast<float>(size);
	float fStep = 0.075f;
	_pIrradianceMap = pComputeCtx->createUnorderedAccessCube(size, size, nullptr, DXGI_FORMAT_R16G16B16A16_FLOAT);

	pComputeCtx->setComputePSO(_pConvolutionIrradiancePSO);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 1);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fStep, 2);
	pComputeCtx->setShaderResourceView(_pEnvMap->getSRV(), SR_EnvMap);
	pComputeCtx->setUnorderedAccessView(_pIrradianceMap->get2DArrayUAV(), UA_Output);

	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	pComputeCtx->dispatch(groupX, groupY, 6);

	auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	pComputeCtx->transitionBarrier(_pIrradianceMap, state);
}

struct SH3Coeff {
	float3 m[9];
};

void IBL::buildIrradianceMapSH(dx12lib::ComputeContextProxy pComputeCtx) {
	constexpr size_t kLightProbeSampleCount = 20000;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	std::vector<float2> consumeStructuredBuffer;
	consumeStructuredBuffer.reserve(kLightProbeSampleCount);
	for (size_t i = 0; i < kLightProbeSampleCount; ++i) {
		float u = dis(gen);
		float v = dis(gen);
		consumeStructuredBuffer.emplace_back(u, v);
	}

	auto pConsumeStructuredBuffer = pComputeCtx->createConsumeStructuredBuffer(consumeStructuredBuffer.data(), kLightProbeSampleCount, sizeof(float2));
	auto pAppendStructuredBuffer = pComputeCtx->createAppendStructuredBuffer(kLightProbeSampleCount, sizeof(SH3Coeff));
	auto pLightProbeReadBack = pComputeCtx->createReadBackBuffer(kLightProbeSampleCount, sizeof(SH3Coeff));

	pComputeCtx->setComputePSO(_pIrradianceMapToSHPSO);
	pComputeCtx->setShaderResourceView(_pIrradianceMap->getSRV(), 0);
	pComputeCtx->setUnorderedAccessView(pConsumeStructuredBuffer->getUAV(), 1);
	pComputeCtx->setUnorderedAccessView(pAppendStructuredBuffer->getUAV(), 2);
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(kLightProbeSampleCount) / kThreadCount));
	pComputeCtx->dispatch(groupX, 1, 1);

	pComputeCtx->copyResource(pLightProbeReadBack, pAppendStructuredBuffer);
	pLightProbeReadBack->setCompletedCallback([&](dx12lib::IReadBackBuffer *pResource) {
		std::memset(&_irradianceMapSH3, 0, sizeof(_irradianceMapSH3));
		Vector3 coeffs[9];
		for (size_t i = 0; i < 9; ++i)
			coeffs[i] = Vector3(0.f);

		const auto *pReadBack = static_cast<dx12lib::ReadBackBuffer *>(pResource);
		std::span<const SH3Coeff> probeCoeffs = pReadBack->visit<SH3Coeff>();
		for (size_t i = 0; i < kLightProbeSampleCount; ++i) {
			coeffs[0] += Vector3(probeCoeffs[i].m[0]);
			coeffs[1] += Vector3(probeCoeffs[i].m[1]);
			coeffs[2] += Vector3(probeCoeffs[i].m[2]);
			coeffs[3] += Vector3(probeCoeffs[i].m[3]);
			coeffs[4] += Vector3(probeCoeffs[i].m[4]);
			coeffs[5] += Vector3(probeCoeffs[i].m[5]);
			coeffs[6] += Vector3(probeCoeffs[i].m[6]);
			coeffs[7] += Vector3(probeCoeffs[i].m[7]);
			coeffs[8] += Vector3(probeCoeffs[i].m[8]);
		}

		constexpr float kNormalizeCoeff = (4.f * DX::XM_PI) / kLightProbeSampleCount;
		for (size_t i = 0; i < 9; ++i)
			_irradianceMapSH3._m[i] = float4(coeffs[i] * kNormalizeCoeff);
	});

	pComputeCtx->trackResource(std::move(pConsumeStructuredBuffer));
	pComputeCtx->trackResource(std::move(pAppendStructuredBuffer));
	pComputeCtx->trackResource(std::move(pLightProbeReadBack));
}


}

