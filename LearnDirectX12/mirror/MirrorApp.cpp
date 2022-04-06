#include <iostream>
#include "MirrorApp.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/ShaderResourceBuffer.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/ContextProxy.hpp"
#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/RenderTargetBuffer.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/UnorderedAccessBuffer.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/window.h"
#include "InputSystem/Mouse.h"
#include "GameTimer/GameTimer.h"
#include "D3D/D3DDescHelper.h"
#include "D3D/FXAA.h"
#include <DirectXColors.h>

Vertex::Vertex(const com::Vertex &vertex)
	: position(vertex.position), normal(vertex.normal), texcoord(vertex.texcoord) {
}

Vertex::Vertex(const float3 &position, const float3 &normal, const float2 &texcoord)
	: position(position), normal(normal), texcoord(texcoord) {
}

MirrorApp::MirrorApp() {
	_title = "Mirror";
}

MirrorApp::~MirrorApp() {

}

void MirrorApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	buildCamera();
	buildConstantBuffers(pDirectCtx);
	loadTextures(pDirectCtx);
	buildMaterials();
	buildMeshs(pDirectCtx);
	buildPSOs(pDirectCtx);
	buildRenderItems(pDirectCtx);
	_pFXAAFilter = std::make_unique<d3d::FXAA>(
		pDirectCtx,
		_width,
		_height,
		_pSwapChain->getRenderTargetFormat()
	);
}

void MirrorApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	while (auto event = _pInputSystem->pMouse->getEvent()) {
		_pCamera->pollEvent(event);
	}

	_pCamera->update(pGameTimer);
	_pCamera->updatePassCB(_pPassCB);
	auto pGPUPassCB = _pPassCB->map();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pGPUPassCB->renderTargetSize = pRenderTarget->getRenderTargetSize();
	pGPUPassCB->invRenderTargetSize = pRenderTarget->getInvRenderTargetSize();
	pGPUPassCB->deltaTime = pGameTimer->getDeltaTime();
	pGPUPassCB->totalTime = pGameTimer->getTotalTime();
	pGPUPassCB->fogColor = float4(0.f);
	pGPUPassCB->fogStart = 0.f;
	pGPUPassCB->fogEnd = 0.f;
	pGPUPassCB->cbPerPassPad0 = 0.f;

}

void MirrorApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pDirectCtx->setViewports(pRenderTarget->getViewport());
	pDirectCtx->setScissorRects(pRenderTarget->getScissiorRect());
	{
		dx12lib::RenderTargetTransitionBarrier barrierGuard = {
			pDirectCtx,
			pRenderTarget,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
		};
		auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(dx12lib::Color0);
		auto pDepthStencilBuffer = pRenderTarget->getDepthStencilBuffer();
		pDirectCtx->clearColor(pRenderTargetBuffer, float4(DX::Colors::LightSkyBlue));
		pDirectCtx->clearDepthStencil(pDepthStencilBuffer, 1.f, 0);
		pDirectCtx->setRenderTarget(pRenderTarget);

		// draw opaque
		auto pPSO = _psoMap[RenderLayer::Opaque];
		pDirectCtx->setGraphicsPSO(pPSO);
		pDirectCtx->setStructuredConstantBuffer(_pPassCB, CBPass);
		pDirectCtx->setStructuredConstantBuffer(_pLightCB, CBLight);
		drawRenderItems(pDirectCtx, RenderLayer::Opaque);

		// mark stencil 
		pPSO = _psoMap[RenderLayer::Mirrors];
		pDirectCtx->setGraphicsPSO(pPSO);
		pDirectCtx->setStructuredConstantBuffer(_pPassCB, CBPass);
		pDirectCtx->setStructuredConstantBuffer(_pLightCB, CBLight);
		pDirectCtx->setStencilRef(1);
		drawRenderItems(pDirectCtx, RenderLayer::Mirrors);

		// draw 
		pPSO = _psoMap[RenderLayer::Reflected];
		pDirectCtx->setGraphicsPSO(pPSO);
		pDirectCtx->setStructuredConstantBuffer(_pPassCB, CBPass);
		pDirectCtx->setStructuredConstantBuffer(_pReflectedLightCB, CBLight);
		pDirectCtx->setStencilRef(1);
		drawRenderItems(pDirectCtx, RenderLayer::Reflected);

		// draw 
		pPSO = _psoMap[RenderLayer::Transparent];
		pDirectCtx->setGraphicsPSO(pPSO);
		pDirectCtx->setStructuredConstantBuffer(_pPassCB, CBPass);
		pDirectCtx->setStructuredConstantBuffer(_pLightCB, CBLight);
		pDirectCtx->setStencilRef(0);
		drawRenderItems(pDirectCtx, RenderLayer::Transparent);

		pPSO = _psoMap[RenderLayer::Shadow];
		pDirectCtx->setGraphicsPSO(pPSO);
		pDirectCtx->setStructuredConstantBuffer(_pPassCB, CBPass);
		pDirectCtx->setStructuredConstantBuffer(_pLightCB, CBLight);
		pDirectCtx->setStencilRef(0);
		drawRenderItems(pDirectCtx, RenderLayer::Shadow);

		_pFXAAFilter->produce(pDirectCtx, pRenderTargetBuffer);
		pDirectCtx->copyResource(pRenderTargetBuffer, _pFXAAFilter->getOutput());
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void MirrorApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	_pCamera->setAspect(float(width) / float(height));
	_pFXAAFilter->onResize(pDirectCtx, width, height);
}

void MirrorApp::drawRenderItems(dx12lib::DirectContextProxy pDirectCtx, RenderLayer layer) {
	pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto &rItem : _renderItems[layer]) {
		pDirectCtx->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pDirectCtx->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pDirectCtx->setStructuredConstantBuffer(rItem._pObjectCB, CBObject);
		pDirectCtx->setShaderResourceBuffer(rItem._pAlbedoMap, SRAlbedo);
		rItem._submesh.drawIndexdInstanced(pDirectCtx);
	}
}

void MirrorApp::buildCamera() {
	d3d::CameraDesc cameraDesc = {
		float3(-1, 1, -1) * 10.f,
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		1000.f,
		float(_width) / float(_height)
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_mouseWheelSensitivity = 2.f;
}

void MirrorApp::buildConstantBuffers(dx12lib::DirectContextProxy pDirectCtx) {
	_pPassCB = pDirectCtx->createStructuredConstantBuffer<d3d::PassCBType>();
	_pLightCB = pDirectCtx->createStructuredConstantBuffer<d3d::LightCBType>();
	_pReflectedLightCB = pDirectCtx->createStructuredConstantBuffer<d3d::LightCBType>();
	auto pGPULightCB = _pLightCB->map();
	float3 directionLight = float3(0, 1, -1);
	pGPULightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCB->directLightCount = 3;
	pGPULightCB->lights[0].initAsDirectionLight(-float3(0.57735f, -0.57735f, 0.57735f), float3(0.6f, 0.6f, 0.6f));
	pGPULightCB->lights[1].initAsDirectionLight(-float3(-0.57735f, -0.57735f, 0.57735f), float3(0.3f, 0.3f, 0.3f));
	pGPULightCB->lights[2].initAsDirectionLight(-float3(0.0f, -0.707f, -0.707f), float3(0.15f, 0.15f, 0.15f));

	using namespace DirectX;
	XMVECTOR mirrorPlane = XMVectorSet(0.f, 0.f, 1.f, 0.f);
	XMMATRIX mirrorReflected = XMMatrixReflect(mirrorPlane);
	auto pGPUReflectedLightCB = _pReflectedLightCB->map();
	*pGPUReflectedLightCB = *pGPULightCB;
	for (std::size_t i = 0; i < pGPUReflectedLightCB->directLightCount; ++i) {
		float3 directionLight = pGPULightCB->lights[i].direction;
		auto reflectedDirectionLight = XMVector3TransformNormal(directionLight.toVec(), mirrorReflected);
		pGPUReflectedLightCB->lights[i].direction = float3(XMVector3Normalize(reflectedDirectionLight));
	}
}

void MirrorApp::loadTextures(dx12lib::DirectContextProxy pDirectCtx) {
	_textureMap["checkboard.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/checkboard.dds");
	_textureMap["ice.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/ice.dds");
	_textureMap["white1x1.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/white1x1.dds");
	_textureMap["bricks3.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/bricks3.dds");
}

void MirrorApp::buildMaterials() {
	d3d::Material skullMat;
	skullMat.diffuseAlbedo = float4(DX::Colors::White);
	skullMat.roughness = 0.3f;
	skullMat.metallic = 0.2f;

	d3d::Material floorMat;
	floorMat.diffuseAlbedo = float4(DX::Colors::White);
	floorMat.roughness = 0.95f;
	floorMat.metallic = 0.05f;

	d3d::Material bricksMat;
	bricksMat.diffuseAlbedo = float4(DX::Colors::White);
	bricksMat.roughness = 0.25f;
	bricksMat.metallic = 0.05f;

	d3d::Material checkertileMat;
	checkertileMat.diffuseAlbedo = float4(DX::Colors::White);
	checkertileMat.roughness = 0.3f;
	checkertileMat.metallic = 0.07f;

	d3d::Material icemirrorMat;
	icemirrorMat.diffuseAlbedo = float4(1.f, 1.f, 1.f, 0.3f);
	icemirrorMat.roughness = 0.5f;
	icemirrorMat.metallic = 0.1f;

	d3d::Material shadowMat;
	shadowMat.diffuseAlbedo = float4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMat.metallic = 0.00f;
	shadowMat.roughness = 0.0f;

	_materialMap["skullMat"] = skullMat;
	_materialMap["floorMat"] = floorMat;
	_materialMap["bricksMat"] = bricksMat;
	_materialMap["checkertileMat"] = checkertileMat;
	_materialMap["icemirrorMat"] = icemirrorMat;
	_materialMap["shadowMat"] = shadowMat;
}

static std::shared_ptr<d3d::Mesh> buildRoomMesh(dx12lib::DirectContextProxy pDirectCtx) {
	std::array<Vertex, 20> vertices = {
		Vertex(float3(-3.5f, 0.0f, -10.0f), float3(0.0f, 1.0f, 0.0f), float2(0.0f, 4.0f)), // 0 
		Vertex(float3(-3.5f, 0.0f,   0.0f), float3(0.0f, 1.0f, 0.0f), float2(0.0f, 0.0f)),
		Vertex(float3(7.5f, 0.0f,    0.0f), float3(0.0f, 1.0f, 0.0f), float2(4.0f, 0.0f)),
		Vertex(float3(7.5f, 0.0f,  -10.0f), float3(0.0f, 1.0f, 0.0f), float2(4.0f, 4.0f)),
		Vertex(float3(-3.5f, 0.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 2.0f)), // 4
		Vertex(float3(-3.5f, 4.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 0.0f)),
		Vertex(float3(-2.5f, 4.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.5f, 0.0f)),
		Vertex(float3(-2.5f, 0.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.5f, 2.0f)),
		Vertex(float3(2.5f, 0.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 2.0f)), // 8 
		Vertex(float3(2.5f, 4.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 0.0f)),
		Vertex(float3(7.5f, 4.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(2.0f, 0.0f)),
		Vertex(float3(7.5f, 0.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(2.0f, 2.0f)),
		Vertex(float3(-3.5f, 4.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 1.0f)), // 12
		Vertex(float3(-3.5f, 6.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 0.0f)),
		Vertex(float3(7.5f, 6.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(6.0f, 0.0f)),
		Vertex(float3(7.5f, 4.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(6.0f, 1.0f)),
		Vertex(float3(-2.5f, 0.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 1.0f)), // 16
		Vertex(float3(-2.5f, 4.0f,   0.0f), float3(0.0f, 0.0f, -1.0f), float2(0.0f, 0.0f)),
		Vertex(float3(2.5f, 4.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(1.0f, 0.0f)),
		Vertex(float3(2.5f, 0.0f,    0.0f), float3(0.0f, 0.0f, -1.0f), float2(1.0f, 1.0f)),
	};

	std::array<std::int16_t, 30> indices = {
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7,
		8, 9, 10,
		8, 10, 11,
		12, 13, 14,
		12, 14, 15,
		16, 17, 18,
		16, 18, 19
	};

	com::MeshData mesh;
	mesh.vertices.reserve(vertices.size());
	mesh.indices.reserve(indices.size());
	std::transform(vertices.begin(), vertices.end(), std::back_inserter(mesh.vertices), [](const Vertex &vert) {
		return com::Vertex{ vert.position, vert.texcoord, vert.normal };
	});
	std::copy(indices.begin(), indices.end(), std::back_inserter(mesh.indices));

	d3d::SubMesh floorSubmesh = { "floor", 6, 0, 0 };
	d3d::SubMesh wallSubmesh = { "wall", 18, 6, 0 };
	d3d::SubMesh mirrorSubMesh = { "mirror", 6, 24, 0 };
	auto pMesh = d3d::MakeMeshHelper<Vertex>::build(pDirectCtx, mesh);
	pMesh->appendSubMesh(floorSubmesh);
	pMesh->appendSubMesh(wallSubmesh);
	pMesh->appendSubMesh(mirrorSubMesh);
	return pMesh;
}

void MirrorApp::buildMeshs(dx12lib::DirectContextProxy pDirectCtx) {
	com::GometryGenerator gen;
	auto skullMesh = gen.loadObjFile("resource/skull.obj");
	auto pSkullMesh = d3d::MakeMeshHelper<Vertex>::build(pDirectCtx, skullMesh, "skull");
	_meshMap["skullGeo"] = pSkullMesh;
	_meshMap["roomGeo"] = buildRoomMesh(pDirectCtx);
}

void MirrorApp::buildPSOs(dx12lib::DirectContextProxy pDirectCtx) {
	dx12lib::RootSignatureDescHelper desc(d3d::getStaticSamplers());
	desc.resize(4);
	desc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// SRV
	desc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// CBV
	desc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	desc[3].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	auto pRootSignature = _pDevice->createRootSignature(desc);
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	auto pOpaquePSO = _pDevice->createGraphicsPSO("OpaquePSO");
	pOpaquePSO->setRootSignature(pRootSignature);
	pOpaquePSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&Vertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&Vertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&Vertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
	};
	pOpaquePSO->setInputLayout(inputLayout);
	pOpaquePSO->setVertexShader(d3d::compileShader(L"shader/default.hlsl", nullptr, "VS", "vs_5_0"));
	pOpaquePSO->setPixelShader(d3d::compileShader(L"shader/default.hlsl", nullptr, "PS", "ps_5_0"));
	pOpaquePSO->finalize();

	auto pTransparentPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pOpaquePSO->clone("TransparentPSO"));
	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	blendDesc.RenderTarget[0] = d3d::RenderTargetBlendDescHelper(d3d::RenderTargetBlendPreset::ALPHA);
	pTransparentPSO->setBlendState(blendDesc);
	pTransparentPSO->finalize();

	auto pMirrorPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pOpaquePSO->clone("MirrorPSO"));
	CD3DX12_DEPTH_STENCIL_DESC mirrorDSS(D3D12_DEFAULT);
	mirrorDSS.DepthEnable = TRUE;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = TRUE;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;
	mirrorDSS.FrontFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_REPLACE);
	mirrorDSS.BackFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_REPLACE);
	CD3DX12_BLEND_DESC mirrorBD(D3D12_DEFAULT);
	mirrorBD.RenderTarget[0].RenderTargetWriteMask = 0;
	pMirrorPSO->setBlendState(mirrorBD);
	pMirrorPSO->setDepthStencilState(mirrorDSS);
	pMirrorPSO->finalize();

	auto pReflectedPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pOpaquePSO->clone("ReflectedPSO"));
	CD3DX12_BLEND_DESC reflectedBlendDesc(D3D12_DEFAULT);
	reflectedBlendDesc.RenderTarget[0] = d3d::RenderTargetBlendDescHelper(d3d::RenderTargetBlendPreset::ALPHA);
	CD3DX12_DEPTH_STENCIL_DESC reflectedDDS(D3D12_DEFAULT);
	reflectedDDS.StencilEnable = TRUE;
	reflectedDDS.StencilReadMask = 0Xff;
	reflectedDDS.StencilWriteMask = 0xff;
	reflectedDDS.FrontFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_KEEP, D3D12_COMPARISON_FUNC_EQUAL);
	reflectedDDS.BackFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_KEEP, D3D12_COMPARISON_FUNC_EQUAL);
	CD3DX12_RASTERIZER_DESC reflectedRasterizerDesc(D3D12_DEFAULT);
	reflectedRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	reflectedRasterizerDesc.FrontCounterClockwise = true;
	pReflectedPSO->setBlendState(reflectedBlendDesc);
	pReflectedPSO->setDepthStencilState(reflectedDDS);
	pReflectedPSO->setRasterizerState(reflectedRasterizerDesc);
	pReflectedPSO->finalize();

	auto pShadowPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pTransparentPSO->clone("ShadowPSO"));
	CD3DX12_DEPTH_STENCIL_DESC shadowDDS(D3D12_DEFAULT);
	shadowDDS.DepthEnable = true;
	shadowDDS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDDS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDDS.StencilEnable = true;
	shadowDDS.StencilReadMask = 0xff;
	shadowDDS.StencilWriteMask = 0xff;
	shadowDDS.FrontFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_INCR, D3D12_COMPARISON_FUNC_EQUAL);
	shadowDDS.BackFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_INCR, D3D12_COMPARISON_FUNC_EQUAL);
	pShadowPSO->setDepthStencilState(shadowDDS);
	pShadowPSO->finalize();

	_psoMap[RenderLayer::Opaque] = pOpaquePSO;
	_psoMap[RenderLayer::Mirrors] = pMirrorPSO;
	_psoMap[RenderLayer::Transparent] = pTransparentPSO;
	_psoMap[RenderLayer::Reflected] = pReflectedPSO;
	_psoMap[RenderLayer::Shadow] = pShadowPSO;
}

void MirrorApp::buildRenderItems(dx12lib::DirectContextProxy pDirectCtx) {
	RenderItem floorRItem;
	ObjectCBType floorObjectCB;
	auto pRoomGeo = _meshMap["roomGeo"];
	floorObjectCB.matWorld = Math::MathHelper::identity4x4();
	floorObjectCB.matNormal = Math::MathHelper::identity4x4();
	floorObjectCB.material = _materialMap["floorMat"];
	floorRItem._pMesh = pRoomGeo;
	floorRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(floorObjectCB);
	floorRItem._pAlbedoMap = _textureMap["checkboard.dds"];
	floorRItem._submesh = pRoomGeo->getSubmesh("floor");
	_renderItems[RenderLayer::Opaque].push_back(floorRItem);

	RenderItem wallsRItem;
	ObjectCBType wallObjectCB;
	wallObjectCB.matWorld = Math::MathHelper::identity4x4();
	wallObjectCB.matNormal = Math::MathHelper::identity4x4();
	wallObjectCB.material = _materialMap["bricksMat"];
	wallsRItem._pAlbedoMap = _textureMap["bricks3.dds"];
	wallsRItem._pMesh = pRoomGeo;
	wallsRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(wallObjectCB);
	wallsRItem._submesh = pRoomGeo->getSubmesh("wall");
	_renderItems[RenderLayer::Opaque].push_back(wallsRItem);

	RenderItem skullRItem;
	ObjectCBType skullObjectCB;
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f * DX::XM_PI);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(_skullTranslation.x, _skullTranslation.y, _skullTranslation.z);
	XMMATRIX skullWorld = skullScale * skullRotate * skullOffset;
	DX::XMStoreFloat4x4(&skullObjectCB.matWorld, skullWorld);
	skullObjectCB.matNormal = Math::MathHelper::identity4x4();
	skullObjectCB.material = _materialMap["skullMat"];
	skullRItem._pMesh = _meshMap["skullGeo"];
	skullRItem._submesh = skullRItem._pMesh->getSubmesh("skull");
	skullRItem._pAlbedoMap = _textureMap["white1x1.dds"];
	skullRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(skullObjectCB);
	_pSkullObjectCB = skullRItem._pObjectCB;
	_renderItems[RenderLayer::Opaque].push_back(skullRItem);


	XMVECTOR mirrorPlane = DX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	XMMATRIX matMirrorPlaneRelfect = DX::XMMatrixReflect(mirrorPlane);
	ObjectCBType reflectedSkullCB = skullObjectCB;
	XMMATRIX reflectSkullWorld = skullWorld * matMirrorPlaneRelfect;
	DX::XMStoreFloat4x4(&reflectedSkullCB.matWorld, reflectSkullWorld);
	auto det = DX::XMMatrixDeterminant(reflectSkullWorld);
	DX::XMStoreFloat4x4(&reflectedSkullCB.matNormal, DX::XMMatrixTranspose(DX::XMMatrixInverse(&det, reflectSkullWorld)));
	RenderItem reflectedSkullRItem = skullRItem;
	reflectedSkullRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(reflectedSkullCB);
	_renderItems[RenderLayer::Reflected].push_back(reflectedSkullRItem);

	RenderItem shadowedSkullRItem = skullRItem;
	ObjectCBType shadowSkullCB = skullObjectCB;
	auto mainLightDirection = _pLightCB->cmap()->lights[0].direction;
	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, _skullTranslation.y * 2); // xz plane
	XMVECTOR toMainLight = mainLightDirection.toVec();
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);
	XMStoreFloat4x4(&shadowSkullCB.matWorld, shadowOffsetY * S * skullWorld);
	auto matZero = XMMatrixSet(
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f
	);
	XMStoreFloat4x4(&shadowSkullCB.matNormal, matZero);
	shadowSkullCB.material = _materialMap["shadowMat"];
	shadowedSkullRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(shadowSkullCB);
	_renderItems[RenderLayer::Shadow].push_back(shadowedSkullRItem);

	RenderItem mirrorRItem;
	ObjectCBType mirrorObjectCB;
	mirrorObjectCB.material = _materialMap["icemirrorMat"];
	mirrorObjectCB.matWorld = Math::MathHelper::identity4x4();
	mirrorObjectCB.matNormal = Math::MathHelper::identity4x4();
	mirrorRItem._pAlbedoMap = _textureMap["ice.dds"];
	mirrorRItem._pMesh = pRoomGeo;
	mirrorRItem._pObjectCB = pDirectCtx->createStructuredConstantBuffer<ObjectCBType>(mirrorObjectCB);
	mirrorRItem._submesh = pRoomGeo->getSubmesh("mirror");
	_renderItems[RenderLayer::Mirrors].push_back(mirrorRItem);
	_renderItems[RenderLayer::Transparent].push_back(mirrorRItem);
}