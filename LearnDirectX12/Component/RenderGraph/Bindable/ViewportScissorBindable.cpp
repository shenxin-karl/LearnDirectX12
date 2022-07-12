#include "ViewportScissorBindable.h"

namespace rg {

ViewportScissorBindable::ViewportScissorBindable(const D3D12_VIEWPORT &viewport, const D3D12_RECT &scissorRect)
: Bindable(BindableType::ViewportScissor), _scissorRect(scissorRect)
, _viewport(viewport)
{
}

void ViewportScissorBindable::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	graphicsCtx.setViewport(_viewport);
	graphicsCtx.setScissorRect(_scissorRect);
}

}
