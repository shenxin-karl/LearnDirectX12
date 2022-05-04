#pragma once
#include "dx12lib/dx12libStd.h"
#include "dx12lib/ContextProxy.hpp"

namespace d3d {

class CubeRenderTarget {
public:
	CubeRenderTarget(dx12lib::CommonContextProxy pCommonCtx, size_t width, size_t height, DXGI_FORMAT format);
};

}
