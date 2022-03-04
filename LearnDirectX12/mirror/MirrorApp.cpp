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
	auto pPSO = _psoMap[layer];
	pCmdList->setPipelineStateObject(pPSO);
	pCmdList->setStructConstantBuffer(_pPassCB, CBPass);
	pCmdList->setStructConstantBuffer(_pLightCB, CBLight);
	for (auto &rItem : _renderItems[layer]) {
		pCmdList->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pCmdList->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pCmdList->setStructConstantBuffer(rItem._pObjectCB, CBObject);
		pCmdList->setShaderResourceView(rItem._pAlbedoMap, SRAlbedo);
		rItem._pMesh->drawIndexdInstanced(pCmdList);
	}
}

void MirrorApp::buildCamera() {
	d3d::CameraDesc cameraDesc = {
		float3(1, 1, -1) * 20.f,
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		1000.f,
		float(_width) / float(_height)
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_whellSensitivety = 5.f;
}

void MirrorApp::buildConstantBuffers(dx12lib::CommandListProxy pCmdList) {
	_pPassCB = pCmdList->createStructConstantBuffer<d3d::PassCBType>();
	_pLightCB = pCmdList->createStructConstantBuffer<d3d::LightCBType>();
	auto pGPULightCb = _pLightCB->map();
	pGPULightCb->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCb->directLightCount = 1;
	pGPULightCb->lights[0].initAsDirectionLight(float3(3, 3, 3), float3(1.f));
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

