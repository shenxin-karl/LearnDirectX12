#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include "SphericalHarmonics.hpp"

namespace d3d {

class IBL {
public:
	IBL(dx12lib::DirectContextProxy pDirectCtx, const std::string &fileName);
	std::shared_ptr<dx12lib::UnorderedAccessCube> getEnvMap() const;
	std::shared_ptr<dx12lib::UnorderedAccessCube> getIrradianceMap() const;
	const SH3 &getIrradianceMapSH3() const;
private:
	void buildSphericalHarmonics3(const std::string &fileName);
	void buildPanoToCubeMapPSO(std::weak_ptr<dx12lib::Device> pDevice);
	void buildIrradianceMapToSHPSO(std::weak_ptr<dx12lib::Device> pDevice);
	void buildConvolutionIrradiancePSO(std::weak_ptr<dx12lib::Device> pDevice);
	void buildEnvMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap);
	void buildConvolutionIrradianceMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pPannoEnvMap);
	void buildIrradianceMapSH(dx12lib::ComputeContextProxy pComputeCtx);
private:
	enum RootParameter : size_t {
		CB_Settings,
		SR_EnvMap,
		UA_Output,
	};
private:
	constexpr static size_t kThreadCount = 32;
	SH3 _irradianceMapSH3;
	std::shared_ptr<dx12lib::ComputePSO>  _pPanoToCubeMapPSO;
	std::shared_ptr<dx12lib::ComputePSO>  _pConvolutionIrradiancePSO;
	std::shared_ptr<dx12lib::ComputePSO>  _pIrradianceMapToSHPSO;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pBRDFLut;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pPerFilteredEnvMap;
	std::shared_ptr<dx12lib::UnorderedAccessCube>  _pEnvMap;
	std::shared_ptr<dx12lib::UnorderedAccessCube>  _pIrradianceMap;
};

}
