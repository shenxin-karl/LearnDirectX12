#pragma once
#include <vector>
#include <memory>
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Step {
public:
	explicit Step(std::shared_ptr<SubPass> pTargetSubPass);
	void addBindable(std::shared_ptr<Bindable> pBindable);
	void submit(const Drawable &drawable) const;
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const;
private:
	std::shared_ptr<SubPass> _pTargetSubPass;
	std::vector<std::shared_ptr<Bindable>> _bindables;
	std::shared_ptr<GraphicsPSOBindable> _pGraphicsPSO;
};

}
