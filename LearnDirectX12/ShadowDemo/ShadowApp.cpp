#include "ShadowApp.h"
#include "D3D/AssimpLoader/ALTree.h"
#include "Dx12lib/Context/CommandQueue.h"
#include "D3D/AssimpLoader/AssimpLoader.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "D3D/Tool/Camera.h"
#include "Dx12lib/Device/SwapChain.h"
#include "InputSystem/Mouse.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "Dx12lib/Pipeline/RootSignature.h"
#include "InputSystem/Keyboard.h"
#include "Dx12lib/Pipeline/PipelineStateObject.h"
#include "Dx12lib/Pipeline/RootSignature.h"
#include "RenderGraph/Bindable/ConstantBufferBindable.h"
#include "RenderGraph/Bindable/GraphicsPSOBindable.h"
#include "RenderGraph/Bindable/SamplerTextureBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Pass/RenderQueuePass.h"
#include "RenderGraph/Technique/Technique.h"
#include "D3D/AssimpLoader/ALTree.h"


void OpaquePass::link(dx12lib::ICommonContext &commonCtx) const {
	RenderQueuePass::link(commonCtx);
	pShadowMap.link(commonCtx);
}

void OpaquePass::reset() {
	RenderQueuePass::reset();
	pShadowMap.reset();;
}

ShadowMaterial::ShadowMaterial(const std::string &name, d3d::INode *pNode, d3d::RenderItem *pRenderItem)
: Material(name, pNode, pRenderItem)
{

}

void ShadowMaterial::initializePso(dx12lib::DirectContextProxy pDirectCtx) {

}

void ShadowMaterial::destroyPso() {
	pOpaquePSO = nullptr;
	pShadowPSO = nullptr;
}

ShadowApp::ShadowApp() {
	_title = "ShadowApp";
	_width = 1280;
	_height = 760;
}

ShadowApp::~ShadowApp() {
}

void ShadowApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::CameraDesc cameraDesc {
		float3(0, 0, 200),
		float3(0, 1, 0),
		float3(-20, 0, 0),
		45.f,
		0.1f,
		1000.f,
		static_cast<float>(_width) / static_cast<float>(_height),
	};
	_pCamera = std::make_shared<d3d::FirstPersonCamera>(cameraDesc);
	_pCamera->_cameraMoveSpeed = 35.f;

	_pPassCb = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pLightCb = pDirectCtx->createConstantBuffer<d3d::CBLightType>();

	auto lightVisitor = _pLightCb->visit<d3d::CBLightType>();
	lightVisitor->ambientLight = float4(0.3f, 0.3f, 0.3f, 1.f);
	lightVisitor->lights[0].initAsDirectionLight(float3(-3, 6, -3), float3(0.8f));
	lightVisitor->lights[1].initAsDirectionLight(float3(-3, +6, -3), float3(0.1f));
	lightVisitor->lights[2].initAsDirectionLight(float3(-3, -6, -3), float3(0.1f));

	D3D12_CLEAR_VALUE shadowMapClearValue;
	shadowMapClearValue.Format = DXGI_FORMAT_D16_UNORM;
	shadowMapClearValue.DepthStencil.Depth = 0.f;
	shadowMapClearValue.DepthStencil.Stencil = 0;
	_pShadowMap = pDirectCtx->createDepthStencil2D(1024, 1024, &shadowMapClearValue, DXGI_FORMAT_D16_UNORM);

	loadModel(pDirectCtx);
	buildPass();
	ShadowMaterial::initializePso(pDirectCtx);
}

void ShadowApp::onDestroy() {
	ShadowMaterial::destroyPso();
}

void ShadowApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	// poll mouse event
	while (auto event = _pInputSystem->pMouse->getEvent()) {
		if (event.isLPress()) {
			_bMouseLeftPress = true;
			_pCamera->setLastMousePosition(POINT(event.x, event.y));
		} else if (event.isLRelease()) {
			_bMouseLeftPress = false;
		}

		if (_bMouseLeftPress || !event.isMove())
			_pCamera->pollEvent(event);
	}

	// poll Keyboard event
	while (auto event = _pInputSystem->pKeyboard->getKeyEvent())
		_pCamera->pollEvent(event);

	_pCamera->update(pGameTimer);
	auto pPassCbVisitor = _pPassCb->visit();
	_pCamera->updatePassCB(*pPassCbVisitor);

}

void ShadowApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	for (auto &pass : _passes)
		pass->execute(pDirectCtx);
	pCmdQueue->executeCommandList(pDirectCtx);
}

void ShadowApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	std::shared_ptr<dx12lib::CommandQueue> pCmdQueue = _pDevice->getCommandQueue();
	pCmdQueue->signal(_pSwapChain);
	for (auto &pass : _passes)
		pass->reset();
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

void ShadowApp::loadModel(dx12lib::DirectContextProxy pDirectCtx) {
	std::shared_ptr<d3d::ALTree> pALTree = std::make_shared<d3d::ALTree>("./resources/powerplant/powerplant.gltf");
	_pMeshModel = std::make_shared<d3d::MeshModel>(*pDirectCtx, pALTree);
}

void ShadowApp::buildPass() {
	auto pClearPass = std::make_shared<rg::ClearPass>("ClearRTAndDS");
	auto pClearShadowMap = std::make_shared<rg::ClearDsPass>("ClearShadowMap");
	auto pShadowPass = std::make_shared<rg::RenderQueuePass>("ShadowPass");
	auto pOpaquePass = std::make_shared<OpaquePass>("OpaquePass");
	auto pPresentPass = std::make_shared<rg::PresentPass>("PresentPass");

	{ // clear RenderTarget and DepthStencil Pass
		auto getRenderTarget = [&]() {
			return _pSwapChain->getRenderTarget2D();
		};
		auto getDepthStencil = [&]() {
			return _pSwapChain->getDepthStencil2D();
		};

		getRenderTarget >> pClearPass->pRenderTarget;
		getDepthStencil >> pClearPass->pDepthStencil;
		_passes.push_back(pClearPass);
	}
	{ // clear Shadow Map
		pClearShadowMap->pDepthStencil.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		_pShadowMap >> pClearShadowMap->pDepthStencil;
		_passes.push_back(pClearShadowMap);
	}
	{ // shadow pass
		pShadowPass->pDepthStencil >> pShadowPass->pDepthStencil;
		_passes.push_back(pShadowPass);
	}
	{ // opaque pass
		pOpaquePass->pShadowMap.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_READ;
		pClearPass->pRenderTarget >> pOpaquePass->pRenderTarget;
		pClearPass->pDepthStencil >> pOpaquePass->pDepthStencil;
		pClearPass->pDepthStencil >> pOpaquePass->pShadowMap;
		_passes.push_back(pOpaquePass);
	}
	{ // Present Pass
		pPresentPass = std::make_shared<rg::PresentPass>("PresentPass");
		pOpaquePass->pRenderTarget >> pPresentPass->pRenderTarget;
		_passes.push_back(pPresentPass);
	}
}
