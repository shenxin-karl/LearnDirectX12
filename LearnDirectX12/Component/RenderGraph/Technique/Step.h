#pragma once
#include <vector>
#include <memory>
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Step {
public:
	explicit Step(std::shared_ptr<RenderQueuePass> pTargetPass);
	void addBindable(std::shared_ptr<Bindable> pBindable);
	void submit(const Drawable &drawable) const;
	void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const;
private:
	std::shared_ptr<RenderQueuePass> _pTargetPass;
	std::vector<std::shared_ptr<Bindable>> _bindables;
};

}
