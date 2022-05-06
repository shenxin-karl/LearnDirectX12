#include "IBL.h"
#include "D3D/Shader/D3DShaderResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <random>
#include <dx12lib/Device/Device.h>
#include <dx12lib/Pipeline/PipelineStd.h>
#include <dx12lib/Texture/TextureStd.h>

#include "D3D/d3dutil.h"
#include "D3D/Shader/ShaderCommon.h"

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

IBL::IBL(dx12lib::ComputeContextProxy pComputeCtx, const std::string &fileName) {
	buildSphericalHarmonics3(fileName);
	buildConvertToCubeMapPso(pComputeCtx->getDevice());

	// todo ×ª»»Îª wstring
	//std::wstring wcharFileName = fileName;
	//std::wstring_view suffix = L".hdr";
	//if (auto iter = wcharFileName.find_last_of(suffix); iter != std::wstring::npos)
	//	wcharFileName.replace(iter, iter + suffix.length(), L".dds");
	//else {
	//	assert(false);
	//}
	
	//auto pEquirecatangular = pComputeCtx->createDDSTexture2DFromFile(wcharFileName);
	//buildEnvMap(pComputeCtx, pEquirecatangular);

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pComputeCtx->createDDSTexture2DFromMemory(brdfLutFile.begin(), brdfLutFile.size());
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

	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplerDesc;
	staticSamplerDesc.push_back(d3d::getPointClampStaticSampler(0));

	dx12lib::RootSignatureDescHelper rootDesc(staticSamplerDesc);
	rootDesc.resize(3);
	rootDesc[CB_Settings].InitAsConstants(2, 0);
	rootDesc[SR_EnvMap].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootDesc[UA_Output].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 6, 0);
	auto pRootSignature = pSharedDevice->createRootSignature(rootDesc);

	_pConvertToCubeMapPSO = pSharedDevice->createComputePSO("ConvertToCubeMapPSO");
	_pConvertToCubeMapPSO->setRootSignature(pRootSignature);

	D3D_SHADER_MACRO macros[] = {
		{ "N", "5" },
		{ nullptr, nullptr },
	};

	cmrc::file shaderContent = d3d::getD3DResource("HlslShader/EquirectangularToCubeMap.hlsl");
	_pConvertToCubeMapPSO->setComputeShader(d3d::compileShader(
		shaderContent.begin(), 
		shaderContent.size(), 
		macros,
		"CSMain", 
		"cs_5_0")
	);
	_pConvertToCubeMapPSO->finalize();
}

void IBL::buildEnvMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pEquirecatangular) {
	size_t width = pEquirecatangular->getWidth();
	size_t height = pEquirecatangular->getHeight();
	size_t size = std::max(width, height) / 6;
	_pEnvMap = pComputeCtx->createUnorderedAccessCube(size, size, nullptr, DXGI_FORMAT_R16G16B16A16_FLOAT);

	pComputeCtx->setComputePSO(_pConvertToCubeMapPSO);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &width, 0);
	pComputeCtx->setCompute32BitConstants(CB_Settings, 1, &height, 1);
	pComputeCtx->setShaderResourceView(pEquirecatangular->getSRV(), SR_EnvMap);
	for (size_t i = 0; i < 6; ++i) {
		dx12lib::CubeFace face = static_cast<dx12lib::CubeFace>(i);
		pComputeCtx->setUnorderedAccessView(_pEnvMap->getFaceUAV(face), SR_EnvMap);
	}
	size_t groupX = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	size_t groupY = static_cast<size_t>(std::ceil(static_cast<float>(size) / kThreadCount));
	pComputeCtx->dispatch(groupX, groupY, 1);
}

}
