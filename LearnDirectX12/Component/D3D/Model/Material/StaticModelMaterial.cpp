#include "StaticModelMaterial.h"
#include "D3D/PSOManager/PSOManager.hpp"
#include "D3D/ShaderManager/ShaderManager.h"

namespace d3d {

const std::string &StaticModelMaterial::getPSOName() const {
	return psoName;
}

void StaticModelMaterial::active(dx12lib::GraphicsContextProxy pGraphicsCtx) {

}

void StaticModelMaterial::initPSOCreator() {
	auto createPSO = [=](std::weak_ptr<dx12lib::Device> pDevice, const std::vector<MacroPair> &macros) {
		auto pSharedDevice = pDevice.lock();
		auto deviceDesc = pSharedDevice->getDesc();
		std::shared_ptr<dx12lib::GraphicsPSO> pso = pSharedDevice->createGraphicsPSO(psoName);
		pso->setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso->setRenderTargetFormat(deviceDesc.backBufferFormat, deviceDesc.depthStencilFormat);
		//pso->setVertexShader(VSShaderManager::get())
		return pso;
	};
	GraphicsPSOManager::instance()->initPSOCreator(psoName, createPSO);
}

}
