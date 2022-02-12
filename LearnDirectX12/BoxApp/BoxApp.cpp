#include "BoxApp.h"
#include "dx12lib/CommandList.h"

BoxApp::BoxApp() {
	_title = "BoxApp";
}

void BoxApp::onInitialize(dx12lib::CommandListProxy pCmdList) {
	_pPassConstantBuffer = pCmdList->createStructConstantBuffer<d3dUtil::PassConstants>();

	CD3DX12_ROOT_SIGNATURE_DESC rootDesc;
}

void BoxApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void BoxApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}
