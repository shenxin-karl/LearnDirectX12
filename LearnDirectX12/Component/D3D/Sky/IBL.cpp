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
	buildSphericalHarmonics3(fileName);
	buildConvertToCubeMapPso(pComputeCtx->getDevice());

	std::wstring wcharFileName = std::to_wstring(fileName);
	std::wstring_view suffix = L".hdr";
	if (auto iter = wcharFileName.find(suffix); iter != std::wstring::npos)
		wcharFileName.replace(iter, iter + suffix.length(), L".dds");
	else {
		assert(false);
	}
	
	_pEquirecatangular = pComputeCtx->createDDSTexture2DFromFile(wcharFileName);
	buildEnvMap(pComputeCtx, _pEquirecatangular);

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pComputeCtx->createDDSTexture2DFromMemory(brdfLutFile.begin(), brdfLutFile.size());
}

std::shared_ptr<dx12lib::UnorderedAccessCube> IBL::getEnvMap() const {
	return _pEnvMap;
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

void IBL::buildConvertToCubeMapPso(std::weak_ptr<dx12lib::Device> pDevice) {
	auto pSharedDevice = pDevice.lock();

	std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	samplers.push_back(d3d::getLinearClampStaticSampler(0));
	dx12lib::RootSignatureDescHelper desc(samplers);
	desc.resize(3);
	desc[CB_Settings].InitAsConstants(2, 0, 0);
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

void IBL::buildEnvMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pEquirecatangular) {
	size_t width = pEquirecatangular->getWidth();
	size_t height = pEquirecatangular->getHeight();
	size_t size = std::max(width, height) / 6;

	float fWidth = static_cast<float>(width);
	float fHeight = static_cast<float>(height);
	float fSize = static_cast<float>(size);
	_pEnvMap = pComputeCtx->createUnorderedAccessCube(size, size, nullptr, DXGI_FORMAT_R16G16B16A16_FLOAT);
	pComputeCtx->setComputePSO(_pPanoToCubeMapPSO);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 0);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &fSize, 1);
	pComputeCtx->setShaderResourceView(pEquirecatangular->getSRV(), SR_EnvMap);
	pComputeCtx->setUnorderedAccessView(_pEnvMap->get2DArrayUAV(), UA_Output);
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	pComputeCtx->dispatch(groupX, groupY, 6);

	auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	pComputeCtx->transitionBarrier(_pEnvMap, state);
}

}
