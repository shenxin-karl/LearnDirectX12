#include "BindingPass.h"
#include "dx12lib/Texture/DepthStencilTexture.h"
#include "dx12lib/Texture/RenderTargetTexture.h"
#include "RenderGraph/Bindable/Bindable.hpp"

namespace rg {

void BindingPass::addBind(std::shared_ptr<Bindable> pBindable) {
	_bindables.push_back(std::move(pBindable));
}

void BindingPass::bindAll(dx12lib::IGraphicsContext &graphicsCtx) const {
	for (auto &pBindable : _bindables)
		pBindable->bind(graphicsCtx);
}

void BindingPass::bindRenderTarget(dx12lib::IGraphicsContext &graphicsCtx) const {
	if (pRenderTarget != nullptr) {
		graphicsCtx.setRenderTarget(
			pRenderTarget->getRTV(renderTargetMipmap),
			pDepthStencil->getDSV()
		);
	} else {
		graphicsCtx.setRenderTarget(pDepthStencil->getDSV());
	}
}

std::shared_ptr<Bindable> BindingPass::getBindableByType(::rg::BindableType bindableType) const {
	for (auto &pBindable : _bindables) {
		if (pBindable->getBindableType() == bindableType)
			return pBindable;
	}
	return nullptr;
}

}
