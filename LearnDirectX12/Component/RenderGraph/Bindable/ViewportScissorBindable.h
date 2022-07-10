#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.h>

namespace rg {

class ViewportScissorBindable : public Bindable {
public:
	ViewportScissorBindable(const D3D12_VIEWPORT &viewport, const D3D12_RECT &scissorRect);
	void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const override;
private:
	D3D12_RECT _scissorRect;
	D3D12_VIEWPORT _viewport;
};

}
