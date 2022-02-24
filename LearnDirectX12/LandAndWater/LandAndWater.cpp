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
#include "dx12lib/RootSignature.h"
#include "dx12lib/PipelineStateObject.h"

LandAndWater::LandAndWater() {
	_title = "LandAndWater";
}

LandAndWater::~LandAndWater() {
}

void LandAndWater::onInitialize(dx12lib::CommandListProxy pCmdList) {
	buildCamera();
	buildConstantBuffer(pCmdList);
	buildLandPSO(pCmdList);
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
	auto pGPULightCB = _pLightCB->map();
	pGPULightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCB->directLightCount = 1;
	pGPULightCB->lights[0].initAsDirectionLight(float3(0.2, 0.8, 0.2), float3(1.f));
}

void LandAndWater::buildLandPSO(dx12lib::CommandListProxy pCmdList) {
	dx12lib::RootSignatureDescHelper desc(d3dutil::getStaticSamplers());
	desc.resize(4);
	desc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// cbv
	desc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	desc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	desc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// srv
	auto pRootSignature = _pDevice->createRootSignature(desc);
	auto pLandPSO = _pDevice->createGraphicsPSO("landPSO");
	pLandPSO->setRootSignature(pRootSignature);
	pLandPSO->setVertexShader(d3dutil::compileShader(L"shader/land.hlsl", nullptr, "VS", "vs_5_0"));
	pLandPSO->setPixelShader(d3dutil::compileShader(L"shader/land.hlsl", nullptr, "PS", "ps_5_0"));
	pLandPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&LandVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&LandVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&LandVertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
	};
	pLandPSO->setInputLayout(inputLayout);
	pLandPSO->finalize();
	_psoMap["LandPso"] = pLandPSO;
}

void LandAndWater::buildWaterPSO(dx12lib::CommandListProxy pCmdList) {

}

