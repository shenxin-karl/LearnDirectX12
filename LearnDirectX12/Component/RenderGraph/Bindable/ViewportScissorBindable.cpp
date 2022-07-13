#include "ViewportScissorBindable.h"

namespace rg {

std::shared_ptr<ViewportScissorBindable> ViewportScissorBindable::make(const D3D12_VIEWPORT &viewport,
	const D3D12_RECT &scissorRect, 
	const std::source_location &sourceLocation)
{
	auto pBindable = std::make_shared<ViewportScissorBindable>(sourceLocation);
	pBindable->_viewport = viewport;
	pBindable->_scissorRect = _scissorRect;
	return pBindable;
}

void ViewportScissorBindable::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	graphicsCtx.setViewport(_viewport);
	graphicsCtx.setScissorRect(_scissorRect);
}

}
