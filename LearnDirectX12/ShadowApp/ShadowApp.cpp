#include "ShadowApp.h"
#include "Context/CommandQueue.h"
#include "D3D/AssimpLoader/AssimpLoader.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "D3D/Tool/Camera.h"
#include "InputSystem/Mouse.h"
#include "dx12lib/Buffer/BufferStd.h"


ShadowApp::ShadowApp() {
	_title = "ShadowApp";
	_width = 1280;
	_height = 760;
}

void ShadowApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::CameraDesc cameraDesc {
		float3(0, 0, 0),
		float3(0, 1, 0),
		float3(0, 0, 1),
		45.f,
		0.1f,
		200.f,
		static_cast<float>(_width) / static_cast<float>(_height),
	};
	_pCamera = std::make_shared<d3d::FirstPersonCamera>(cameraDesc);
	_pPassCb = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pLightCb = pDirectCtx->createConstantBuffer<d3d::CBLightType>();

	loadModel(pDirectCtx);
}

void ShadowApp::onDestroy() {

}

void ShadowApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	// poll event
	while (com::MouseEvent event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);

	auto pPassCbVisitor = _pPassCb->visit();
	_pCamera->updatePassCB(*pPassCbVisitor);
}

void ShadowApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{
		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectCtx);
		renderTarget.clear(pDirectCtx);
		renderTarget.unbind(pDirectCtx);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
}

void ShadowApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	std::shared_ptr<dx12lib::CommandQueue> pCmdQueue = _pDevice->getCommandQueue();
	pCmdQueue->signal(_pSwapChain);
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

void ShadowApp::loadModel(dx12lib::DirectContextProxy pDirectCtx) {
	auto loadModelImpl = [&](const std::string &name, const std::string &path) {
		d3d::AssimpLoader loader(path, true);
		assert(loader.isLoad());
	};
	loadModelImpl("Cathedral", "resources/Cathedral.glb");
	loadModelImpl("QuaintVillag", "resources/Quaint Village.glb");
	loadModelImpl("TreeHouse", "resources/Tree House.glb");
}

void ShadowApp::buildRenderItem() const {

}
