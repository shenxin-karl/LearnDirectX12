#include "GraphicsPSOBindable.h"
#include "dx12lib/Pipeline/PipelineStateObject.h"
#include "D3D/PSOManager/PSOManager.h"

namespace rg {

GraphicsPSOBindable::GraphicsPSOBindable(const Pass *pPass, const std::string &psoName) : Bindable(BindableType::PipelineStateObject) {
	auto *pGraphicsPSOManager = d3d::GraphicsPSOManager::instance();
	if (pGraphicsPSOManager->exist(psoName)) {
		_pso = pGraphicsPSOManager->get(psoName);
		return;
	}

}

void GraphicsPSOBindable::bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	pGraphicsCtx->setGraphicsPSO(_pso);
}

const std::string & GraphicsPSOBindable::getPSOName() const {
	return _pso->getName();
}

}
