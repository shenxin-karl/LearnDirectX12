#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include "SphericalHarmonics.hpp"

namespace d3d {

class IBL {
public:
	IBL(dx12lib::ComputeContextProxy pComputeCtx, const std::string &fileName);
private:
	void buildSphericalHarmonics3(const std::string &fileName);
	void buildConvertToCubeMapPso(std::weak_ptr<dx12lib::Device> pDevice);
	void buildEnvMap(dx12lib::ComputeContextProxy pComputeCtx, std::shared_ptr<dx12lib::IShaderResource2D> pEquirecatangular);
private:
	enum RootParameter : size_t {
		CB_Settings,
		SR_EnvMap,
		UA_Output,
	};
private:
	constexpr static size_t kThreadCount = 32;
	SH3 _irradianceMapSH3;
	std::shared_ptr<dx12lib::ComputePSO> _pConvertToCubeMapPSO;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pBRDFLut;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pPerFilteredEnvMap;
	std::shared_ptr<dx12lib::UnorderedAccessCube> _pEnvMap;
};

}
