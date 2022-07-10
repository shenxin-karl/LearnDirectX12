#include "RenderQueuePass.h"

namespace rg {

void RenderQueuePass::accept(Job job) {
	_jobs.push_back(job);
}

void RenderQueuePass::execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	bindAll(pGraphicsCtx);
	bindRenderTarget(pGraphicsCtx);
	for (auto &job : _jobs)
		job.execute(pGraphicsCtx);
}

void RenderQueuePass::reset() {
	BindingPass::reset();
	_jobs.clear();
}

}
