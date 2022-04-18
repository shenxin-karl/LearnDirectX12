#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/ContextProxy.hpp"
#include "SphericalHarmonics.hpp"

namespace d3d {

class IBL {
public:
	IBL(dx12lib::GraphicsContextProxy pGraphicsCtx, const std::string &fileName);
private:
private:
	SH3 _irradianceMapSH3;
	std::shared_ptr<dx12lib::ShaderResourceBuffer> _pBRDFLut;
	std::shared_ptr<dx12lib::ShaderResourceBuffer> _pPerFilteredEnvMap;
};

}
