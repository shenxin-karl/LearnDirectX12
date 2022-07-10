#include "Step.h"
#include "RenderGraph/Bindable/Bindable.h"
#include "RenderGraph/Drawable/Drawable.h"

namespace rg {

Step::Step(std::shared_ptr<RenderQueuePass> pTargetPass) : _pTargetPass(std::move(pTargetPass)) {
	assert(_pTargetPass != nullptr);
}

void Step::addBindable(std::shared_ptr<Bindable> pBindable) {
	_bindables.push_back(std::move(pBindable));
}

void Step::submit(const Drawable &drawable) const {
	assert(_pTargetPass != nullptr);
	// todo
}

void Step::bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	for (auto &pBindable : _bindables)
		pBindable->bind(pGraphicsCtx);
}

}
