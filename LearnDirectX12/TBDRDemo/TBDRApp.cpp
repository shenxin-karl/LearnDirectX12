#include "TBDRApp.h"
#include "D3D/Model/MeshModel/MeshModel.h"

TBDRApp::TBDRApp() {
}

TBDRApp::~TBDRApp() {
}

void TBDRApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	auto pAlTree = std::make_shared<d3d::ALTree>("resources/SponzaPBR/Sponza.gltf");
	_pMeshModel = std::make_unique<d3d::MeshModel>(*pDirectCtx, pAlTree);
}

void TBDRApp::onDestroy() {
}

void TBDRApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onBeginTick(pGameTimer);
}

void TBDRApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onTick(pGameTimer);
}

void TBDRApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::onEndTick(pGameTimer);
}

void TBDRApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

auto TBDRApp::getSwapChain() const -> std::shared_ptr<dx12lib::SwapChain> {
	return _pSwapChain;
}
