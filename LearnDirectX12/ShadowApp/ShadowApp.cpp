#include "ShadowApp.h"
#include "Context/CommandQueue.h"
#include "D3D/AssimpLoader/AssimpLoader.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "D3D/Model/StaticModel/StaticModel.h"
#include "D3D/Tool/Camera.h"
#include "Device/SwapChain.h"
#include "InputSystem/Mouse.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "Pipeline/PipelineStateObject.h"
#include "Pipeline/RootSignature.h"
#include "RenderGraph/Bindable/ConstantBufferBindable.h"
#include "RenderGraph/Bindable/GraphicsPSOBindable.h"
#include "RenderGraph/Bindable/SamplerTextureBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Pass/RenderQueuePass.h"
#include "RenderGraph/Technique/Technique.h"


Node::Node(dx12lib::IGraphicsContext &graphicsCtx, std::shared_ptr<d3d::StaticModel> pStaticModel)
: _pStaticModel(std::move(pStaticModel))
{
	_pCbObject = graphicsCtx.createFRConstantBuffer<CbObjectType>(CbObjectType());
	for (size_t i = 0; i < _pStaticModel->getSubModelCount(); ++i) {
		auto pSubModel = std::static_pointer_cast<d3d::StaticSubModel>(_pStaticModel->getSubModel(i));
		auto pDrawable = std::make_unique<rg::Drawable>();
		pDrawable->setVertexBuffer(pSubModel->getVertexBuffer());
		pDrawable->setIndexBuffer(pSubModel->getIndexBuffer());
		pDrawable->setTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDrawable->genDrawArgs();
		_drawables.push_back(std::move(pDrawable));
	}
}

void Node::buildOpaqueTechnique(std::shared_ptr<rg::SubPass> pSubPass) const {
	auto pCbv0Bindable = std::make_shared<rg::ConstantBufferBindable>(
		dx12lib::RegisterSlot::CBV0,
		_pCbObject
	);

	for (size_t i = 0; i < _drawables.size(); ++i) {
		const auto &material = _pStaticModel->getSubModel(i)->getMaterial();
		const auto &albedoMapName = material.getalbedoMapName();
		auto pAlbedoMap = d3d::TextureManager::instance()->get(albedoMapName);
		auto pSrv1Bindable = std::make_shared<rg::SamplerTextureBindable>(
			dx12lib::RegisterSlot::SRV1,
			pAlbedoMap
		);

		auto pTechnique = std::make_unique<rg::Technique>("OpaqueTechnique", rg::TechniqueType::Color);
		auto pStep = std::make_unique<rg::Step>(pSubPass);
		pStep->addBindable(pCbv0Bindable);
		pStep->addBindable(pSrv1Bindable);
		pTechnique->addStep(std::move(pStep));
		_drawables[i]->addTechnique(std::move(pTechnique));
	}
}

void Node::buildShadowTechnique(std::shared_ptr<rg::SubPass> pSubPass) const {
	auto pCbv0Bindable = std::make_shared<rg::ConstantBufferBindable>(
		dx12lib::RegisterSlot::CBV0,
		_pCbObject
	);

	for (size_t i = 0; i < _drawables.size(); ++i) {
		auto pTechnique = std::make_unique<rg::Technique>("ShadowTechnique", rg::TechniqueType::Shadow);
		auto pStep = std::make_unique<rg::Step>(pSubPass);
		pStep->addBindable(pCbv0Bindable);
		pTechnique->addStep(std::move(pStep));
		_drawables[i]->addTechnique(std::move(pTechnique));
	}
}

ShadowApp::ShadowApp() {
	_title = "ShadowApp";
	_width = 1280;
	_height = 760;
}

void ShadowApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	_pd3dInitializer = std::make_unique<d3d::D3DInitializer>();
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
	_pd3dInitializer.reset();
}

void ShadowApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	// poll event
	while (com::MouseEvent event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);

	auto pPassCbVisitor = _pPassCb->visit();
	_pCamera->updatePassCB(*pPassCbVisitor);

	_pOpaquePass->pRenderTarget = _pSwapChain->getRenderTarget2D();
	_pOpaquePass->pDepthStencil = _pSwapChain->getDepthStencil2D();
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
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

void ShadowApp::loadModel(dx12lib::DirectContextProxy pDirectCtx) {
	auto loadModelImpl = [&](const std::string &name, const std::string &path) {
		d3d::AssimpLoader loader(path);
		loader.load();
		assert(loader.isLoad());
		auto pModel = std::make_shared<d3d::StaticModel>(name);
		pModel->initAsAssimpLoader(pDirectCtx, loader);
		_modelMap[name] = std::move(pModel);
	};
	loadModelImpl("Cathedral", "resources/Cathedral.glb");
	loadModelImpl("QuaintVillag", "resources/Quaint Village.glb");
	loadModelImpl("TreeHouse", "resources/Tree House.glb");
}

void ShadowApp::buildPass() {
	const auto &initDesc = _pDevice->getDesc();
	{
		_pOpaquePass = std::make_shared<rg::RenderQueuePass>("OpaquePass");
		_pOpaquePass->setDSVFormat(initDesc.depthStencilFormat);
		_pOpaquePass->setRTVFormats({ initDesc.backBufferFormat });
	}
	{
		_pShadowPass = std::make_shared<rg::RenderQueuePass>("ShadowPass");
		_pShadowPass->pDepthStencil = _pShadowMap;
		_pShadowPass->setDSVFormat(_pShadowMap->getFormat());
	}
}

void ShadowApp::buildPSOAndSubPass() {
	auto pPassCbBindable = std::make_shared<rg::ConstantBufferBindable>(
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
		blinnPhongPSO->setBlendState(_pOpaquePass->getBlendDesc());
		blinnPhongPSO->setRasterizerState(_pOpaquePass->getRasterizerState());
		blinnPhongPSO->setDepthStencilState(_pOpaquePass->getDepthStencilDesc());
		blinnPhongPSO->setRenderTargetFormats(_pOpaquePass->getRTVFormats(), _pOpaquePass->getDSVFormat());
		blinnPhongPSO->setPrimitiveTopologyType(_pOpaquePass->getPrimitiveTopologyType());
		blinnPhongPSO->setInputLayout(d3d::StaticSubModel::getInputLayout());
		blinnPhongPSO->finalize();

		auto pLightCbBindable = std::make_shared<rg::ConstantBufferBindable>(
			dx12lib::RegisterSlot::CBV2,
			_pLightCb
		);

		auto pBlinnPhongPsoBindable = std::make_shared<rg::GraphicsPSOBindable>(blinnPhongPSO);
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
		shadowPSO->setBlendState(_pOpaquePass->getBlendDesc());
		shadowPSO->setRasterizerState(_pOpaquePass->getRasterizerState());
		shadowPSO->setDepthStencilState(_pOpaquePass->getDepthStencilDesc());
		shadowPSO->setRenderTargetFormats(_pOpaquePass->getRTVFormats(), _pOpaquePass->getDSVFormat());
		shadowPSO->setPrimitiveTopologyType(_pOpaquePass->getPrimitiveTopologyType());
		shadowPSO->setInputLayout(d3d::StaticSubModel::getInputLayout());
		shadowPSO->finalize();

		auto pShadowPsoBindable = std::make_shared<rg::GraphicsPSOBindable>(shadowPSO);
		auto pSubPass = _pShadowPass->getOrCreateSubPass(pShadowPsoBindable);
		pSubPass->addBindable(pPassCbBindable);
	}
}

void ShadowApp::buildNodes() {

}

