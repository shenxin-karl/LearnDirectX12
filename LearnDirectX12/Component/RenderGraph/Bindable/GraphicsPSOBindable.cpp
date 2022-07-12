#include "GraphicsPSOBindable.h"
#include "dx12lib/Pipeline/PipelineStateObject.h"
#include "D3D/PSOManager/PSOManager.hpp"

namespace rg {

GraphicsPSOBindable::GraphicsPSOBindable(std::shared_ptr<dx12lib::GraphicsPSO> pso)
: Bindable(BindableType::PipelineStateObject), _pso(pso)
{
}

void GraphicsPSOBindable::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	graphicsCtx.setGraphicsPSO(_pso);
}

const std::string & GraphicsPSOBindable::getPSOName() const {
	return _pso->getName();
}

}
