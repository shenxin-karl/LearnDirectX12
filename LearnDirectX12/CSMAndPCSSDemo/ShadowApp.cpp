#include "ShadowApp.h"
#include "ShadowRgph.h"
#include "ShadowMaterial.h"
#include "D3D/AssimpLoader/ALTree.h"
#include "Dx12lib/Context/CommandQueue.h"
#include "D3D/AssimpLoader/AssimpLoader.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "D3D/Tool/Camera.h"
#include "Dx12lib/Device/SwapChain.h"
#include "InputSystem/Mouse.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "InputSystem/Keyboard.h"
#include "RenderGraph/Bindable/SamplerTextureBindable.h"
#include "RenderGraph/Drawable/Drawable.h"
#include "RenderGraph/Technique/Technique.h"
#include "D3D/Shadow/CSMShadowPass.h"
#include "D3D/Tool/FirstPersonCamera.h"

using namespace Math;


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
		1024.f,
		static_cast<float>(_width) / static_cast<float>(_height),
	};
	_pCamera = std::make_shared<d3d::FirstPersonCamera>(cameraDesc);
	_pCamera->_cameraMoveSpeed = 60.f;

	_pPassCb = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pLightCb = pDirectCtx->createConstantBuffer<d3d::CBLightType>();

	auto lightVisitor = _pLightCb->visit<d3d::CBLightType>();
	lightVisitor->ambientLight = float4(0.2f, 0.2f, 0.2f, 1.f);
	lightVisitor->lights[0].initAsDirectionLight(float3(-3, 6, -3), float3(3.f));
	lightVisitor->lights[1].initAsDirectionLight(float3(-3, +6, -3), float3(0.1f));
	lightVisitor->lights[2].initAsDirectionLight(float3(-3, -6, -3), float3(0.1f));

	_pEnvMap = pDirectCtx->createDDSTextureCubeFromFile(L"resources/grasscube1024.dds");
	_pRenderGraph = createShadowRenderGraph(this, pDirectCtx);
	_pCSMShadowPass = dynamic_cast<d3d::CSMShadowPass *>(
		_pRenderGraph->getRenderQueuePass(ShadowRgph::ShadowPass)
	);
	assert(_pCSMShadowPass != nullptr);
	_pCSMShadowPass->setZMulti(10.f);
	_pCSMShadowPass->setLightSize(10.f);
	_pCSMShadowPass->setLightPlane(300.f);

	ShadowMaterial::init(this);

	std::shared_ptr<d3d::ALTree> pALTree = std::make_shared<d3d::ALTree>("./resources/powerplant/powerplant.gltf");
	_pMeshModel = std::make_shared<d3d::MeshModel>(*pDirectCtx, pALTree);
	_pMeshModel->createMaterial(*_pRenderGraph,
		*pDirectCtx,
		ShadowMaterial::getShadowMaterialCreator(pDirectCtx)
	);
	_pMeshModel->setModelTransform(static_cast<float4x4>(Matrix4::makeScale(2.f)));
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

	auto pLightCbVisitor = _pLightCb->visit<d3d::CBLightType>();
	Vector3 lightDir(pLightCbVisitor->lights[0].direction);
	_lightBoundingBox = _pCSMShadowPass->update(_pCamera.get(), pGameTimer, -lightDir);
}

void ShadowApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	_pMeshModel->submit(d3d::MakeBoundingWrap(_lightBoundingBox), ShadowRgph::kShadow);
	_pMeshModel->submit(d3d::MakeBoundingWrap(_pCamera->getViewSpaceFrustum()), ShadowRgph::kOpaque);
	_pRenderGraph->execute(pDirectCtx);
	pCmdQueue->executeCommandList(pDirectCtx);
}

void ShadowApp::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	std::shared_ptr<dx12lib::CommandQueue> pCmdQueue = _pDevice->getCommandQueue();
	pCmdQueue->signal(_pSwapChain);
	_pRenderGraph->reset();
}

void ShadowApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	_pCamera->setAspect(aspect);
}
