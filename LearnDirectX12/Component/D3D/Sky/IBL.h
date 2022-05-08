#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include "SphericalHarmonics.hpp"

namespace d3d {

class IBL {
public:
	IBL(dx12lib::DirectContextProxy pDirectCtx, const std::string &fileName);
	std::shared_ptr<dx12lib::RenderTargetCube> getEnvMap() const;
private:
	void buildSphericalHarmonics3(const std::string &fileName);
	void buildConvertToCubeMapPso(std::weak_ptr<dx12lib::Device> pDevice);
	void buildEnvMap(dx12lib::DirectContextProxy pDirectCtx, std::shared_ptr<dx12lib::IShaderResource2D> pEquirecatangular);
private:
	enum RootParameter : size_t {
		CB_Settings,
		SR_EnvMap,
	};
private:
	constexpr static size_t kThreadCount = 32;
	SH3 _irradianceMapSH3;
	std::shared_ptr<dx12lib::GraphicsPSO> _pConvertToCubeMapPSO;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pBRDFLut;
	std::shared_ptr<dx12lib::SamplerTexture2D> _pPerFilteredEnvMap;
	std::shared_ptr<dx12lib::IShaderResource2D> _pEquirecatangular;
	std::shared_ptr<dx12lib::RenderTargetCube>  _pEnvMap;
};

}
