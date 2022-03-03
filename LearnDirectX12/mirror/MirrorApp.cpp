#include "MirrorApp.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/window.h"
#include "GameTimer/GameTimer.h"

MirrorApp::MirrorApp() {
	_title = "Mirror";
}

MirrorApp::~MirrorApp() {

}

void MirrorApp::onInitialize(dx12lib::CommandListProxy pCmdList) {
	buildCamera();
	buildConstantBuffers(pCmdList);
	loadTextures(pCmdList);
	buildMaterials();
	buildMeshs(pCmdList);
	buildPSOs();
	buildRenderItems();
}

void MirrorApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void MirrorApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void MirrorApp::onResize(dx12lib::CommandListProxy pCmdList, int width, int height) {
	_pCamera->_aspect = float(width) / float(height);
}

void MirrorApp::drawRenderItems(dx12lib::CommandListProxy pCmdList, RenderLayer layer) {

}

void MirrorApp::buildCamera() {

}

void MirrorApp::buildConstantBuffers(dx12lib::CommandListProxy pCmdList) {

}

void MirrorApp::loadTextures(dx12lib::CommandListProxy pCmdList) {

}

void MirrorApp::buildMaterials() {

}

void MirrorApp::buildMeshs(dx12lib::CommandListProxy pCmdList) {

}

void MirrorApp::buildPSOs() {

}

void MirrorApp::buildRenderItems() {

}

