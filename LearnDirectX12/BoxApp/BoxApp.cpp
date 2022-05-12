#include "BoxApp.h"

#include "D3D/d3dutil.h"
#include "D3D/Tool/Mesh.h"
#include "D3D/dx12libHelper/RenderTarget.h"
#include "D3D/Sky/IBL.h"

#include "InputSystem/mouse.h"
#include <DirectXColors.h>
#include <dx12lib/Device/DeviceStd.h>
#include <dx12lib/Context/ContextStd.h>
#include <dx12lib/Texture/TextureStd.h>
#include <dx12lib/Buffer/BufferStd.h>
#include <dx12lib/Pipeline/PipelineStd.h>

#include "D3D/Sky/SkyBox.h"

BoxApp::BoxApp() {
	_title = "BoxApp";
}

void BoxApp::onInitialize(dx12lib::DirectContextProxy pDirectContext) {
	d3d::CameraDesc cameraDesc;
	cameraDesc.lookAt = float3(0, 0, 0);
	cameraDesc.lookFrom = float3(5, 0, 0);
	cameraDesc.lookUp = float3(0, 1, 0);
	cameraDesc.nearClip = 0.1f;
	cameraDesc.farClip = 100.f;
	cameraDesc.fov = 45.f;
	cameraDesc.aspect = float(_width) / float(_height);

	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCBObject = pDirectContext->createFRConstantBuffer<CBObject>();
	_pCBObject->visit()->gMaterial = {float4(1.f), 0.5, 0.5f};



	// initialize root signature
	dx12lib::RootSignatureDescHelper desc(d3d::getStaticSamplers());
	desc.resize(2);
	desc[CB_Object].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	desc[SR_Env].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	auto pRootSignature = _pDevice->createRootSignature(desc);

	// initialize graphics pipeline state object
	_pGraphicsPSO = _pDevice->createGraphicsPSO("colorPSO");
	_pGraphicsPSO->setVertexShader(d3d::compileShader(L"shader/Color.hlsl", nullptr, "VS", "vs_5_0"));
	_pGraphicsPSO->setPixelShader(d3d::compileShader(L"shader/Color.hlsl", nullptr, "PS", "ps_5_0"));
	_pGraphicsPSO->setRootSignature(pRootSignature);
	_pGraphicsPSO->setRenderTargetFormats(
		1, RVPtr(_pSwapChain->getRenderTargetFormat()),
		_pSwapChain->getDepthStencilFormat()
	);

	// input layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&Vertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0),
		dx12lib::VInputLayoutDescHelper(&Vertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0),
	};
	_pGraphicsPSO->setInputLayout(inputLayout);
	_pGraphicsPSO->finalize();

	buildBoxGeometry(pDirectContext);
}

void BoxApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	auto pObject = _pCBObject->map();
	_pCamera->update(pGameTimer);

	Matrix4 rotate = Matrix4::makeZRotationByRadian(pGameTimer->getTotalTime());
	Matrix4 viewProj = _pCamera->getMatViewProj();
	Matrix4 translation = Matrix4::makeTranslation(0.f, std::cos(pGameTimer->getTotalTime()), std::sin(pGameTimer->getTotalTime()));
	Matrix4 matWorld = translation * rotate;
	pObject->gMatWorldViewProj = float4x4(viewProj);
	pObject->gMatNormal = float4x4(Matrix4::identity()); //float4x4(rotate);
	//pObject->gSH3 = _pIBL->getIrradianceMapSH3();
}

void BoxApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectProxy = pCmdQueue->createDirectContextProxy();
	{
		_pIBL = std::make_unique<d3d::IBL>(pDirectProxy, "resources/Barce_Rooftop_C_3k.hdr");
		d3d::SkyBoxDesc skyBoxDesc = {
			.pGraphicsCtx = pDirectProxy,
			.pCubeMap = _pIBL->getEnvMap(),
			.renderTargetFormat = _pSwapChain->getRenderTargetFormat(),
			.depthStencilFormat = _pSwapChain->getDepthStencilFormat(),
		};
		_pSkyBox = std::make_unique<d3d::SkyBox>(skyBoxDesc);

		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectProxy);
		float cosine = std::cos(pGameTimer->getTotalTime());
		float sine = std::sin(pGameTimer->getTotalTime());
		float4 color = {
			cosine * 0.5f + 0.5f,
			sine * 0.5f + 0.5f,
			0.6f,
			1.f
		};
		renderTarget.clear(pDirectProxy, color);
		renderBoxPass(pDirectProxy);
		_pSkyBox->render(pDirectProxy, _pCamera);
		renderTarget.unbind(pDirectProxy);
	}
	pCmdQueue->executeCommandList(pDirectProxy);
	pCmdQueue->signal(_pSwapChain);
}

void BoxApp::resize(int width, int height) {
	_pCamera->setAspect(static_cast<float>(width) / static_cast<float>(height));
}

void BoxApp::pollEvent() {
	while (auto event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);
}

void BoxApp::buildBoxGeometry(dx12lib::DirectContextProxy pDirectContext) {
	com::GometryGenerator gen;
	com::MeshData cubeSphere = gen.createCubeSphere(1.f, 3);
	_pBoxMesh = std::make_unique<BoxMesh>();
	auto makeMeshHelper = d3d::MakeMeshHelper<Vertex>();
	_pBoxMesh->_pVertexBuffer = makeMeshHelper.buildVertexBuffer(pDirectContext, cubeSphere);
	_pBoxMesh->_pIndexBuffer = makeMeshHelper.buildIndexBuffer(pDirectContext, cubeSphere);
	_pBoxMesh->_baseVertexLocation = 0;
	_pBoxMesh->_startIndexLocation = 0;
}

void BoxApp::renderBoxPass(dx12lib::DirectContextProxy pDirectContext) const {
	pDirectContext->setGraphicsPSO(_pGraphicsPSO);
	pDirectContext->setConstantBuffer(_pCBObject, WorldViewProjCBuffer, 0);
	pDirectContext->setVertexBuffer(_pBoxMesh->_pVertexBuffer);
	pDirectContext->setIndexBuffer(_pBoxMesh->_pIndexBuffer);
	pDirectContext->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDirectContext->setShaderResourceView(_pIBL->getIrradianceMap()->getSRV(), SR_Env, 0);
	pDirectContext->drawIndexedInstanced(
		_pBoxMesh->_pIndexBuffer->getIndexCount(),
		1,
		_pBoxMesh->_baseVertexLocation,
		_pBoxMesh->_startIndexLocation,
		0
	);
}
