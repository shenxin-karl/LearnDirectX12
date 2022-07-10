#include "BindingPass.h"

#include "dx12lib/Texture/DepthStencilTexture.h"
#include "dx12lib/Texture/RenderTargetTexture.h"
#include "RenderGraph/Bindable/Bindable.h"

namespace rg {

void BindingPass::addBind(std::shared_ptr<Bindable> pBindable) {
	_bindables.push_back(std::move(pBindable));
}

void BindingPass::bindAll(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	for (auto &pBindable : _bindables)
		pBindable->bind(pGraphicsCtx);
}

void BindingPass::bindRenderTarget(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	if (pRenderTarget != nullptr) {
		pGraphicsCtx->setRenderTarget(
			pRenderTarget->getRTV(renderTargetMipmap),
			pDepthStencil->getDSV()
		);
	} else {
		pGraphicsCtx->setRenderTarget(pDepthStencil->getDSV());
	}
}

}
