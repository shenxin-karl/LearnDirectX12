#include "BasicTessellation.h"
#include "RenderTarget.h"
#include "SwapChain.h"
#include "D3D/Camera.h"
#include "D3D/Mesh.h"
#include "D3D/ShaderCommon.h"
#include "dx12lib/Device.h"
#include "dx12lib/PipelineStateObject.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include "Math/MathHelper.h"

using namespace Math;

BasicTessellationApp::BasicTessellationApp() : BaseApp() {
}

BasicTessellationApp::~BasicTessellationApp() {
}

void BasicTessellationApp::onInitialize(dx12lib::DirectContextProxy pCmdList) {
	d3d::CameraDesc cameraDesc = {
		float3(300.f),
		float3(0.f, 1.f, 0.f),
		float3(0.f),
		45.f,
		0.1f,
		1000.f,
		static_cast<float>(_width) / static_cast<float>(_height)
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);

	_pTessellationPSO = _pDevice->createGraphicsPSO("TessellationPSO");
	_pTessellationPSO->setVertexShader(d3d::compileShader(
		L"shader/Tessellation.hlsl", 
		nullptr, 
		"VS", 
		"vs_5_0")
	);
	_pTessellationPSO->setHullShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"HS",
		"hs_5_0"
	));
	_pTessellationPSO->setDomainShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"DS",
		"ds_5_0"
	));
	_pTessellationPSO->setPixelShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"PS",
		"ps_5_0"
	));
	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	_pTessellationPSO->setRasterizerState(rasterizerDesc);
	_pTessellationPSO->finalize();


}

void BasicTessellationApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	while (auto event = _pInputSystem->mouse->getEvent())
		_pCamera->pollEvent(event);

	_pCamera->update();
	_pCamera->updatePassCB(_pPassCB);
	auto pPassCBuffer = _pPassCB->map();
	pPassCBuffer->totalTime = pGameTimer->getTotalTime();
	pPassCBuffer->deltaTime = pGameTimer->getDeltaTime();
	auto pRenderTarget = _pSwapChain->getRenderTarget();;
	pPassCBuffer->renderTargetSize = pRenderTarget->getRenderTargetSize();
	pPassCBuffer->invRenderTargetSize = pRenderTarget->getInvRenderTargetSize();
}

void BasicTessellationApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void BasicTessellationApp::onResize(dx12lib::DirectContextProxy pCmdList, int width, int height) {
	_pCamera->_aspect = static_cast<float>(width) / static_cast<float>(height);
}
