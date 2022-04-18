#include "IBL.h"
#include "D3DShaderResource.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <random>
#include "dx12lib/Device.h"

namespace d3d {



template<typename T>
SH3 calcIrradianceMapSH3(const T *pData, size_t width, size_t height, size_t numSamples) {
	SH3 coef;
	for (auto &c : coef._m)
		c = float3(0.f);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.f, 1.f);
	auto sh3BasisFuncList = SH3::getSHBasisFunc();
	constexpr float invPdf = 4.f * XM_PI;

	size_t maxX = width - 1;
	size_t maxY = height - 1;
	size_t sampleCount = 0;
	for (size_t i = 0; i < numSamples; ++i) {
		float u = dis(gen);
		float v = dis(gen);
		float theta = 2.f * std::acos(std::sqrt(1.f - u));
		float phi = 2.f * XM_PI * v;
		float cosTh = std::cos(theta);
		float sinTh = std::sin(theta);
		float cosPhi = std::cos(phi);
		float sinPhi = std::sin(phi);

		float3 L = normalize(float3(sinPhi * cosTh, cosPhi, sinPhi * sinTh));
		size_t x = static_cast<size_t>(u * maxX);
		size_t y = static_cast<size_t>(v * maxY);
		size_t index = y * width + x;
		float3 texColor = float3(pData[index].x, pData[index].y, pData[index].z) * invPdf;
		for (size_t j = 0; j < sh3BasisFuncList.size(); ++j) {
			float c = sh3BasisFuncList[j](L);
			coef._m[j] += c * texColor;
		}
		++sampleCount;
	}

	float invSample = 1.f / static_cast<float>(sampleCount);
	for (auto &c : coef._m)
		c *= invSample;

	return coef;
}

float3 getSHRadian(SH3 lightProbe, float3 N) {
	float3 result = float3(0.f);
	result += lightProbe.y00 * SHBasisFunction<0, 0>::eval(N);
	result += lightProbe.y1_1 * SHBasisFunction<1, -1>::eval(N);
	result += lightProbe.y10 * SHBasisFunction<1, 0>::eval(N);
	result += lightProbe.y11 * SHBasisFunction<1, 1>::eval(N);
	return result;
}

void test(SH3 lightProbe) {
	float3 N0 = float3(1, 0, 0);
	float3 N1 = float3(0, 1, 0);
	float3 N2 = float3(0, 1, 1);

	float3 c0 = getSHRadian(lightProbe, N0);
	float3 c1 = getSHRadian(lightProbe, N1);
	float3 c2 = getSHRadian(lightProbe, N2);
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

	test(_irradianceMapSH3);

	cmrc::file brdfLutFile = getD3DResource("resources/BRDF_LUT.dds");
	_pBRDFLut = pGraphicsCtx->createDDSTextureFromMemory(brdfLutFile.begin(), brdfLutFile.size());
}

}
