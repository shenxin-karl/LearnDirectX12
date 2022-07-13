#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.hpp>

#include "dx12lib/Context/CommonContext.h"

namespace rg {

class ViewportScissorBindable : public Bindable {
public:
	std::shared_ptr<ViewportScissorBindable> make(const D3D12_VIEWPORT &viewport, 
		const D3D12_RECT &scissorRect,
		DECLARE_SOURCE_LOCATION_ARG
	);
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const override;
	ViewportScissorBindable(const std::source_location &sr) : Bindable(BindableType::ViewportScissor, sr) {}
private:
	D3D12_RECT _scissorRect;
	D3D12_VIEWPORT _viewport;
};

}
