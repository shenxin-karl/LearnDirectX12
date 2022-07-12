#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.hpp>

#include "dx12lib/Context/CommonContext.h"

namespace rg {

class ViewportScissorBindable : public Bindable {
public:
	ViewportScissorBindable(const D3D12_VIEWPORT &viewport, const D3D12_RECT &scissorRect);
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const override;
private:
	D3D12_RECT _scissorRect;
	D3D12_VIEWPORT _viewport;
};

}
