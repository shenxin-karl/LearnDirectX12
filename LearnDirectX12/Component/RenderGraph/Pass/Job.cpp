#include "Job.h"
#include "RenderGraph/Bindable/GraphicsPSOBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Technique/Step.h"

namespace rg {

Job::Job(const Step *pStep, const Drawable *pDrawable)
: _pStep(pStep), _pDrawable(pDrawable)
{
	assert(pStep != nullptr);
	assert(pDrawable != nullptr);
}

void Job::execute(dx12lib::IGraphicsContext &graphicsCtx) const {
	_pDrawable->bind(graphicsCtx);
	_pStep->bind(graphicsCtx);

	const auto &drawArgs = _pDrawable->getDrawArgs();
	if (drawArgs.indexCount > 0) {
		graphicsCtx.drawIndexedInstanced(
			drawArgs.indexCount,
			drawArgs.instanceCount,
			drawArgs.startIndexLocation,
			drawArgs.baseVertexLocation,
			drawArgs.startInstanceLocation
		);
	} else {
		graphicsCtx.drawInstanced(
			drawArgs.vertexCount,
			drawArgs.instanceCount,
			drawArgs.baseVertexLocation,
			drawArgs.startInstanceLocation
		);
	}
}

}
