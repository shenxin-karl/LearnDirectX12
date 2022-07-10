#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Job {
public:
	Job(const Drawable *pDrawable, const Step *pStep);
	void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const;
private:
	const Drawable *_pDrawable;
	const Step *_pStep;
};

}
