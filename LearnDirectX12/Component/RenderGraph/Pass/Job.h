#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Job {
public:
	Job(const Step *pStep, const Drawable *pDrawable);
	void execute(dx12lib::IGraphicsContext &graphicsCtx) const;
private:
	const Step *_pStep;
	const Drawable *_pDrawable;
};

}
