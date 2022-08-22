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
	buildPSOAndSubPass();
}

void ShadowApp::onDestroy() {
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

	//rg::TechniqueFlag flag = rg::TechniqueType::Color;
	//for (auto &pNode : _nodes)
		//pNode->submit(flag);
}

void ShadowApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{
		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectCtx);
		renderTarget.clear(pDirectCtx);
		_pOpaquePass->execute(pDirectCtx);
		renderTarget.unbind(pDirectCtx);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
}

void ShadowApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	std::shared_ptr<dx12lib::CommandQueue> pCmdQueue = _pDevice->getCommandQueue();
	pCmdQueue->signal(_pSwapChain);

	_pOpaquePass->reset();
	_pShadowPass->reset();
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

void ShadowApp::loadModel(dx12lib::DirectContextProxy pDirectCtx) {
	std::shared_ptr<d3d::ALTree> pALTree = std::make_shared<d3d::ALTree>("./resources/powerplant/powerplant.gltf");
}

void ShadowApp::buildPass() {
	{ // clear Pass
		
	}
	{ // shadow pass
		
	}
	{ // opaque pass
		_pOpaquePass = std::make_shared<rg::RenderQueuePass>("OpaquePass");
	}
}

void ShadowApp::buildPSOAndSubPass() {
	auto pPassCbBindable = rg::ConstantBufferBindable::make(
		dx12lib::RegisterSlot::CBV1,
		_pPassCb
	); 

	{	/// Build Opaque SubPass
		auto pRootSignature = _pDevice->createRootSignature(2, 6);
		pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 },		// CbObject
			{ dx12lib::RegisterSlot::SRV0, 1 },		// gAlbedoMap
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV1, 1 },		// CbPass
			{ dx12lib::RegisterSlot::CBV2, 1 },		// CbLight
		});
		pRootSignature->finalize();

		auto blinnPhongPSO = _pDevice->createGraphicsPSO("BlinnPhongPSO");
		blinnPhongPSO->setRootSignature(pRootSignature);
		blinnPhongPSO->setVertexShader(d3d::compileShader(L"shaders/BlinnPhong.hlsl", nullptr, "VS", "vs_5_0"));
		blinnPhongPSO->setPixelShader(d3d::compileShader(L"shaders/BlinnPhong.hlsl", nullptr, "PS", "ps_5_0"));

		//blinnPhongPSO->setInputLayout(d3d::StaticSubModel::getInputLayout());
		blinnPhongPSO->finalize();

		auto pLightCbBindable = rg::ConstantBufferBindable::make(
			dx12lib::RegisterSlot::CBV2,
			_pLightCb
		);

		auto pBlinnPhongPsoBindable = rg::GraphicsPSOBindable::make(blinnPhongPSO);
		auto pSubPass = _pOpaquePass->getOrCreateSubPass(pBlinnPhongPsoBindable);
		pSubPass->addBindable(pLightCbBindable);
		pSubPass->addBindable(pPassCbBindable);
	}

	{	/// Build Shadow SubPass
		auto pRootSignature = _pDevice->createRootSignature(2, 0);
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 },		// CbObject
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV1, 1 },		// CbPass
		});
		pRootSignature->finalize();


		auto shadowPSO = _pDevice->createGraphicsPSO("ShadowPSO");
		shadowPSO->setRootSignature(pRootSignature);
		shadowPSO->setVertexShader(d3d::compileShader(L"shaders/Shadows.hlsl", nullptr, "VS", "vs_5_0"));
		shadowPSO->setPixelShader(d3d::compileShader(L"shaders/Shadows.hlsl", nullptr, "PS", "ps_5_0"));
		//shadowPSO->setInputLayout(d3d::StaticSubModel::getInputLayout());
		shadowPSO->finalize();

		auto pShadowPsoBindable = rg::GraphicsPSOBindable::make(shadowPSO);
		auto pSubPass = _pShadowPass->getOrCreateSubPass(pShadowPsoBindable);
		pSubPass->addBindable(pPassCbBindable);
	}
}
