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
#include "Geometry/GeometryGenerator.h"

namespace d3d {

template<typename T>
SH3 calcIrradianceMapSH3(const T *pData, size_t width, size_t height, size_t numSamples) {
	Vector3 shCoefficient[9] = { Vector3(0.f) };

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	auto shBasisFuncArray = SH3::getSHBasisFunc();
	constexpr float invPdf = 4.f * DirectX::XM_PI;

	size_t maxX = width - 1;
	size_t maxY = height - 1;
	size_t sampleCount = 0;
	for (size_t i = 0; i < numSamples; ++i) {
		float u = dis(gen);
		float v = dis(gen);
		float theta = 2.f * std::acos(std::sqrt(1.f - u));
		float phi = 2.f * DirectX::XM_PI * v;
		float cosTh = std::cos(theta);
		float sinTh = std::sin(theta);
		float cosPhi = std::cos(phi);
		float sinPhi = std::sin(phi);

		float3 L = float3(sinPhi * cosTh, cosPhi, sinPhi * sinTh);
		size_t x = static_cast<size_t>(u * maxX);
		size_t y = static_cast<size_t>(v * maxY);
		size_t index = y * width + x;
		Vector3 texColor = Vector3(pData[index].x, pData[index].y, pData[index].z) * invPdf;
		for (size_t j = 0; j < shBasisFuncArray.size(); ++j) {
			float c = shBasisFuncArray[j](L);
			Vector3 basisColor = c * texColor;
			shCoefficient[j] += basisColor;
		}
		++sampleCount;
	}

	SH3 coeff;
	float invSample = 1.f / static_cast<float>(sampleCount);
	for (size_t i = 0; i < 9; ++i) {
		shCoefficient[i] *= invSample;
		coeff._m[i] = float4(shCoefficient[i].xyz, 0.f);
	}

	return coeff;
}

IBL::IBL(dx12lib::DirectContextProxy pComputeCtx, const std::string &fileName) {
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

void IBL::buildSphericalHarmonics3(const std::string &fileName) {
	std::memset(&_irradianceMapSH3, 0, sizeof(SH3));
	int width = 0; int height = 0; int channel = 0;
	float *pHdrMap = stbi_loadf(fileName.c_str(), &width, &height, &channel, 0);
	if (pHdrMap == nullptr) {
		assert(false);
		return;
	}

	switch (channel) {
	case 3:
		_irradianceMapSH3 = calcIrradianceMapSH3(reinterpret_cast<float3 *>(pHdrMap), width, height, 10000);
		break;
	case 4:
		_irradianceMapSH3 = calcIrradianceMapSH3(reinterpret_cast<float4 *>(pHdrMap), width, height, 10000);
		break;
	case 0:
	case 1:
	case 2:
	default:
		assert(false);
	}

	stbi_image_free(pHdrMap);
	pHdrMap = nullptr;
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

void IBL::buildIrradianceMapSH(dx12lib::ComputeContextProxy pComputeCtx) {
	constexpr size_t kLightProbeSampleCount = 20000;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	std::vector<float2> randomNumberPair;
	randomNumberPair.reserve(kLightProbeSampleCount);
	for (std::size_t i = 0; i < kLightProbeSampleCount; ++i)
		randomNumberPair.emplace_back(dis(gen), dis(gen));

	struct SH3Coeff {
		float3 m[9];
	};

	auto pConsumeStructuredBuffer = pComputeCtx->createConsumeStructuredBuffer(kLightProbeSampleCount, sizeof(float2));
	auto pAppendStructuredBuffer = pComputeCtx->createAppendStructuredBuffer(kLightProbeSampleCount, sizeof(SH3Coeff));


}


}

