#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Job {
public:
	Job(const Step *pStep, const Drawable *pDrawable, const GraphicsPSOBindable *pGraphicsPSO);
	void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const;
private:
	const Step *_pStep;
	const Drawable *_pDrawable;
	const GraphicsPSOBindable *_pGraphicsPSO;
};

}
