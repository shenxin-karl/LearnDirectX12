#pragma once
#include <dx12lib/dx12libStd.h>
#include <D3D/d3dutil.h>

namespace d3d {

class PCFShadow {
public:

private:
	std::shared_ptr<dx12lib::DepthStencil2D> _pShadowMap;
};

}
