#include "Job.h"

#include "RenderGraph/Drawable/Drawable.h"

namespace rg {

Job::Job(const Drawable *pDrawable, const Step *pStep) : _pDrawable(pDrawable), _pStep(pStep) {
}

void Job::execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	_pDrawable->bind(pGraphicsCtx);
	_pStep->bind(pGraphicsCtx);
}

}
