#include "Step.h"
#include "RenderGraph/Bindable/Bindable.hpp"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Pass/RenderQueuePass.h"

namespace rg {

Step::Step(std::shared_ptr<SubPass> pTargetSubPass) : _pTargetSubPass(std::move(pTargetSubPass)) {
	assert(_pTargetSubPass != nullptr);
}

void Step::addBindable(std::shared_ptr<Bindable> pBindable) {
	_bindables.push_back(std::move(pBindable));
}

void Step::submit(const Drawable &drawable) const {
	assert(_pTargetSubPass != nullptr);
	_pTargetSubPass->accept(Job{ this, &drawable });
}

void Step::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	for (auto &pBindable : _bindables)
		pBindable->bind(graphicsCtx);
}

}
