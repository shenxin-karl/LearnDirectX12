#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Pass/Pass.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class BindingPass : public Pass {
public:
	using Pass::Pass;
	void addBind(std::shared_ptr<Bindable> pBindable);
	void bindAll(dx12lib::IGraphicsContext &graphicsCtx) const;
	void bindRenderTarget(dx12lib::IGraphicsContext &graphicsCtx) const;
public:
	size_t renderTargetMipmap = 0;
	std::shared_ptr<dx12lib::RenderTarget2D> pRenderTarget;
	std::shared_ptr<dx12lib::DepthStencil2D> pDepthStencil;
private:
	std::vector<std::shared_ptr<Bindable>> _bindables;
};

}
