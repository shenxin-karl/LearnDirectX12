#include "Job.h"
#include "RenderGraph/Bindable/GraphicsPSOBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Technique/Step.h"

namespace rg {

Job::Job(const Step *pStep, const Drawable *pDrawable, const GraphicsPSOBindable *pGraphicsPSO)
: _pStep(pStep), _pDrawable(pDrawable)
, _pGraphicsPSO(pGraphicsPSO)
{
	assert(pStep != nullptr);
	assert(pDrawable != nullptr);
	assert(pGraphicsPSO != nullptr);
}

void Job::execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	_pGraphicsPSO->bind(pGraphicsCtx);
	_pDrawable->bind(pGraphicsCtx);
	_pStep->bind(pGraphicsCtx);

	const auto &drawArgs = _pDrawable->getDrawArgs();
	if (drawArgs.indexCount > 0) {
		pGraphicsCtx->drawIndexedInstanced(
			drawArgs.indexCount,
			drawArgs.instanceCount,
			drawArgs.startIndexLocation,
			drawArgs.baseVertexLocation,
			drawArgs.startInstanceLocation
		);
	} else {
		pGraphicsCtx->drawInstanced(
			drawArgs.vertexCount,
			drawArgs.instanceCount,
			drawArgs.baseVertexLocation,
			drawArgs.startInstanceLocation
		);
	}
}

}
