#include "LandAndWater.h"
#include "GameTimer/GameTimer.h"
#include "d3d/Camera.h"
#include "InputSystem/window.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/Mouse.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/Texture.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/IndexBuffer.h"

LandAndWater::LandAndWater() {
	_title = "LandAndWater";
}

LandAndWater::~LandAndWater() {
}

void LandAndWater::onInitialize(dx12lib::CommandListProxy pCmdList) {
	buildCamera();
	buildConstantBuffer(pCmdList);
	buildPSO(pCmdList);
	buildGeometrys(pCmdList);
	loadTextures(pCmdList);
	buildMaterials();
	buildRenderItems(pCmdList);
}

void LandAndWater::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	_pCamera->update();
	updateConstantBuffer(pGameTimer);
}

void LandAndWater::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	// todo
}

void LandAndWater::onResize(dx12lib::CommandListProxy pCmdList, int width, int height) {
	_pCamera->_aspect = float(width) / float(height);
}

void LandAndWater::pollEvent() {
	while (auto event = _pInputSystem->mouse->getEvent())
		_pCamera->pollEvent(event);
}

void LandAndWater::updateConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer) {
	_pCamera->updatePassCB(_pPassCB);
	auto pGPUPassCB = _pPassCB->map();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pGPUPassCB->renderTargetSize = pRenderTarget->getInvRenderTargetSize();
	pGPUPassCB->invRenderTargetSize = pRenderTarget->getInvRenderTargetSize();
	pGPUPassCB->deltaTime = pGameTimer->getDeltaTime();
	pGPUPassCB->totalTime = pGameTimer->getTotalTime();
}

void LandAndWater::buildCamera() {
	d3dutil::CameraDesc desc = {
		float3(10, 10, 10),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		100.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3dutil::CoronaCamera>(desc);
}

void LandAndWater::buildConstantBuffer(dx12lib::CommandListProxy pCmdList) {
	_pPassCB = pCmdList->createStructConstantBuffer<d3dutil::PassCBType>();
	_pLightCB = pCmdList->createStructConstantBuffer<d3dutil::LightCBType>();
	// todo set directior light
}

