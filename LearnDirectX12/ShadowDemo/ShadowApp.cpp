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
#include "RenderGraph/Bindable/SamplerTextureBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Pass/RenderQueuePass.h"
#include "RenderGraph/Technique/Technique.h"
#include "D3D/AssimpLoader/ALTree.h"
#include "D3D/TextureManager/TextureManager.h"
#include "RenderGraph/Pass/SubPass.h"
#include "D3D/Tool/FirstPersonCamera.h"


ShadowPass::ShadowPass(const std::string &passName) : RenderQueuePass(passName, false, true) {
}

OpaquePass::OpaquePass(const std::string &passName)
: RenderQueuePass(passName)
, pShadowMap(this, "ShadowMap")
{
}

ShadowMaterial::ShadowMaterial(dx12lib::IDirectContext &directCtx, std::shared_ptr<dx12lib::IShaderResource2D> pDiffuseTex)
: rgph::Material("ShadowMaterial")
{
	_vertexInputSlots = pOpaqueSubPass->getVertexDataInputSlots() | pShadowSubPass->getVertexDataInputSlots();
	_pCbObject = directCtx.createFRConstantBuffer<CbObject>(CbObject{});
	auto pAlbedoBindable = rgph::SamplerTextureBindable::make(
		dx12lib::RegisterSlot::SRV0, pDiffuseTex
	);

	auto pCbObjectBindable = rgph::ConstantBufferBindable::make(
		dx12lib::RegisterSlot::CBV1, _pCbObject
	);

	// opaque 
	auto pOpaqueTechnique = std::make_shared<rgph::Technique>("Opaque", TechType::kOpaque);
	{
		auto pStep = std::make_unique<rgph::Step>(this, pOpaqueSubPass.get());
		pStep->addBindable(pAlbedoBindable);
		pStep->addBindable(pCbObjectBindable);
		pOpaqueTechnique->addStep(std::move(pStep));
	}
	_techniques.push_back(pOpaqueTechnique);

	// shadow
	auto pShadowTechnique = std::make_shared<rgph::Technique>("Shadow", TechType::kShadow);
	{
		auto pStep = std::make_unique<rgph::Step>(this, pShadowSubPass.get());
		pShadowTechnique->addStep(std::move(pStep));
	}
	_techniques.push_back(pShadowTechnique);
}


ShadowApp::ShadowApp() {
	_title = "ShadowApp";
	_width = 1280;
	_height = 760;
}

ShadowApp::~ShadowApp() {
}

void ShadowApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	_pSwapChain->setVerticalSync(false);
	d3d::CameraDesc cameraDesc {
		float3(110.045f, 8.51247f, -0.0528324f),
		float3(-4.37114e-08, 1, 0),
		float3(109.05f, 8.41141f, -0.0424141f),
		45.f,
		0.1f,
		1000.f,
		static_cast<float>(_width) / static_cast<float>(_height),
	};
	_pCamera = std::make_shared<d3d::FirstPersonCamera>(cameraDesc);
	_pCamera->_cameraMoveSpeed = 60.f;

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

	buildPass();
	initPso(pDirectCtx);
	initSubPass();
	loadModel(pDirectCtx);
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


}

void ShadowApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	_pMeshModel->submit(_pCamera->getViewSpaceFrustum(), TechType::kOpaque);
	_graph.execute(pDirectCtx);
	pCmdQueue->executeCommandList(pDirectCtx);
}

void ShadowApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	std::shared_ptr<dx12lib::CommandQueue> pCmdQueue = _pDevice->getCommandQueue();
	pCmdQueue->signal(_pSwapChain);
	_graph.reset();
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}

void ShadowApp::loadModel(dx12lib::DirectContextProxy pDirectCtx) {
	std::shared_ptr<d3d::ALTree> pALTree = std::make_shared<d3d::ALTree>("./resources/powerplant/powerplant.gltf");
	_pMeshModel = std::make_shared<d3d::MeshModel>(*pDirectCtx, pALTree);

	auto materialCreator = [&](const d3d::ALMaterial *pAlMaterial) -> std::shared_ptr<rgph::Material> {
		const auto &diffuseMap = pAlMaterial->getDiffuseMap();
		std::shared_ptr<dx12lib::IShaderResource> pTex = d3d::TextureManager::instance()->get(diffuseMap.path);

		if (pTex == nullptr) {
			if (diffuseMap.pTextureData != nullptr) {
				pTex = pDirectCtx->createTextureFromMemory(diffuseMap.textureExtName, 
					diffuseMap.pTextureData.get(), 
					diffuseMap.textureDataSize
				);
			} else {
				pTex = pDirectCtx->createTextureFromFile(std::to_wstring(diffuseMap.path), true);
			}
			d3d::TextureManager::instance()->set(diffuseMap.path, pTex);
		}

		std::shared_ptr<dx12lib::IShaderResource2D> pTex2D = std::dynamic_pointer_cast<dx12lib::IShaderResource2D>(pTex);
		if (pTex2D == nullptr) {
			assert(false && "load diffuse Map Error");
		}
		return std::make_shared<ShadowMaterial>(*pDirectCtx, pTex2D);
	};

	_pMeshModel->createMaterial(_graph, *pDirectCtx, materialCreator);
}

void ShadowApp::initPso(dx12lib::DirectContextProxy pDirectCtx) const {
	/// ShadowMaterial::pOpaquePso
	auto pSharedDevice = pDirectCtx->getDevice().lock();
	{
		auto pRootSignature = pSharedDevice->createRootSignature(2, 6);
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 }, // cbTransform
			{ dx12lib::RegisterSlot::CBV1, 1 }, // cbObject
			{ dx12lib::RegisterSlot::SRV0, 1 }, // gAlbedoMap
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV2, 1 }, // cbPass
			{ dx12lib::RegisterSlot::CBV3, 1 }, // cbLight
		});
		pRootSignature->initStaticSampler(0, d3d::getStaticSamplers());
		pRootSignature->finalize();

		auto pOpaquePso = pSharedDevice->createGraphicsPSO("OpaquePso");
		pOpaquePso->setRenderTargetFormat(_pSwapChain->getRenderTargetFormat(), _pSwapChain->getDepthStencilFormat());
		pOpaquePso->setRootSignature(pRootSignature);
		pOpaquePso->setInputLayout({
			d3d::PositionSemantic,
			d3d::NormalSemantic,
			d3d::Texcoord0Semantic,
		});
		pOpaquePso->setVertexShader(d3d::compileShader(
			L"shaders/BlinnPhong.hlsl", 
			nullptr, 
			"VS", 
			"vs_5_0")
		);
		pOpaquePso->setPixelShader(d3d::compileShader(
			L"shaders/BlinnPhong.hlsl",
			nullptr,
			"PS",
			"ps_5_0"
		));
		pOpaquePso->finalize();
		ShadowMaterial::pOpaquePso = pOpaquePso;
	}
	/// ShadowMaterial::pShadowPso
	{
		auto pRootSignature = pSharedDevice->createRootSignature(2);
		pRootSignature->at(0).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV0, 1 }, // cbTransform
		});
		pRootSignature->at(1).initAsDescriptorTable({
			{ dx12lib::RegisterSlot::CBV1, 1 }, // cbPass
		});
		pRootSignature->finalize();
		auto pShadowPso = pSharedDevice->createGraphicsPSO("ShadowPSO");
		pShadowPso->setRootSignature(pRootSignature);
		pShadowPso->setDepthTargetFormat(_pShadowMap->getFormat());
		pShadowPso->setInputLayout({ d3d::PositionSemantic });
		pShadowPso->setVertexShader(d3d::compileShader(
			L"shaders/Shadows.hlsl",
			nullptr,
			"VS",
			"vs_5_0"
		));
		pShadowPso->setPixelShader(d3d::compileShader(
			L"shaders/Shadows.hlsl",
			nullptr,
			"PS",
			"ps_5_0"
		));
		pShadowPso->finalize();
		ShadowMaterial::pShadowPso = pShadowPso;
	}
}

void ShadowApp::initSubPass() {
	auto pLightCbBindable = rgph::ConstantBufferBindable::make(
		dx12lib::RegisterSlot::CBV3,
		_pLightCb
	);

	/// ShadowMaterial::pOpaqueSubPass
	{
		rgph::VertexInputSlots vertexInputSlots;
		vertexInputSlots.set(d3d::PositionSemantic.slot);
		vertexInputSlots.set(d3d::Texcoord0Semantic.slot);
		vertexInputSlots.set(d3d::NormalSemantic.slot);
		auto pOpaqueSubPass = std::make_shared<rgph::SubPass>(ShadowMaterial::pOpaquePso);
		pOpaqueSubPass->setPassCBufferShaderRegister(dx12lib::RegisterSlot::CBV2);
		pOpaqueSubPass->setTransformCBufferShaderRegister(dx12lib::RegisterSlot::CBV0);
		pOpaqueSubPass->setVertexDataInputSlots(vertexInputSlots);
		pOpaqueSubPass->addBindable(pLightCbBindable);
		ShadowMaterial::pOpaqueSubPass = pOpaqueSubPass;
		_graph.getRenderQueuePass("OpaquePass")->addSubPass(pOpaqueSubPass);
	}
	/// ShadowMaterial::pShadowSubPass
	{
		rgph::VertexInputSlots vertexInputSlots;
		vertexInputSlots.set(d3d::PositionSemantic.slot);
		auto pShadowSubPass = std::make_shared<rgph::SubPass>(ShadowMaterial::pShadowPso);
		pShadowSubPass->setPassCBufferShaderRegister(dx12lib::RegisterSlot::CBV1);
		pShadowSubPass->setTransformCBufferShaderRegister(dx12lib::RegisterSlot::CBV0);
		pShadowSubPass->setVertexDataInputSlots(vertexInputSlots);
		ShadowMaterial::pShadowSubPass = pShadowSubPass;
		_graph.getRenderQueuePass("ShadowPass")->addSubPass(pShadowSubPass);
	}
}

void ShadowApp::buildPass() {
	auto pClearPass = std::make_shared<rgph::ClearPass>("ClearRTAndDS");
	auto pClearShadowMap = std::make_shared<rgph::ClearDsPass>("ClearShadowMap");
	auto pShadowPass = std::make_shared<ShadowPass>("ShadowPass");
	auto pOpaquePass = std::make_shared<OpaquePass>("OpaquePass");
	auto pPresentPass = std::make_shared<rgph::PresentPass>("PresentPass");

	{ // clear RenderTarget and DepthStencil Pass
		auto getRenderTarget = [&]() {
			return _pSwapChain->getRenderTarget2D();
		};
		auto getDepthStencil = [&]() {
			return _pSwapChain->getDepthStencil2D();
		};

		getRenderTarget >> pClearPass->pRenderTarget;
		getDepthStencil >> pClearPass->pDepthStencil;
		_graph.addPass(pClearPass);
	}
	{ // clear Shadow Map
		pClearShadowMap->pDepthStencil.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		_pShadowMap >> pClearShadowMap->pDepthStencil;
		_graph.addPass(pClearShadowMap);
	}
	{ // shadow pass
		pClearShadowMap->pDepthStencil >> pShadowPass->pDepthStencil;
		_graph.addPass(pShadowPass);
	}
	{ // opaque pass
		pOpaquePass->setPassCBuffer(_pPassCb);
		pOpaquePass->pShadowMap.preExecuteState = D3D12_RESOURCE_STATE_DEPTH_READ;
		pClearPass->pRenderTarget >> pOpaquePass->pRenderTarget;
		pClearPass->pDepthStencil >> pOpaquePass->pDepthStencil;
		pShadowPass->pDepthStencil >> pOpaquePass->pShadowMap;
		_graph.addPass(pOpaquePass);
	}
	{ // Present Pass
		pOpaquePass->pRenderTarget >> pPresentPass->pRenderTarget;
		_graph.addPass(pPresentPass);
	}
	_graph.finalize();
}
