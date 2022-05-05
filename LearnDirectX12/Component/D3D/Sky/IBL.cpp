#include "IBL.h"
#include "D3D/Shader/D3DShaderResource.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <random>
#include <dx12lib/Device/Device.h>

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

float3 getSHRadian(SH3 lightProbe, float3 N) {
	Vector4 result(0.f);
	result += Vector4(lightProbe.y0p0) * SHBasisFunction<0, 0>::eval(N);
	result += Vector4(lightProbe.y1n1)* SHBasisFunction<1, -1>::eval(N);
	result += Vector4(lightProbe.y1p0) * SHBasisFunction<1, 0>::eval(N);
	result += Vector4(lightProbe.y1p1) * SHBasisFunction<1, 1>::eval(N);
	return result.xyz;
}

IBL::IBL(dx12lib::GraphicsContextProxy pGraphicsCtx, const std::string &fileName) {
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
		return;
	}

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pGraphicsCtx->createDDSTexture2DFromMemory(brdfLutFile.begin(), brdfLutFile.size());
}

}
