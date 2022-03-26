#include "Shape.h"
#include "D3D/d3dutil.h"
#include "D3D/Camera.h"
#include "D3D/ShaderCommon.h"
#include "D3D/SobelFilter.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/RenderTargetBuffer.h"
#include "dx12lib/ShaderResourceBuffer.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/UnorderedAccessBuffer.h"
#include "Geometry/GeometryGenerator.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Mouse.h"
#include <DirectXColors.h>
#include <iostream>

Shape::Shape() {
	_title = "Shape";
}

Shape::~Shape() {

}

void Shape::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::CameraDesc cameraDesc = {
		float3(10, 10, 10),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		100.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_whellSensitivety = 1.f;
	_pPassCB = pDirectCtx->createStructConstantBuffer<d3d::PassCBType>();
	buildTexturePSO(pDirectCtx);
	buildColorPSO(pDirectCtx);
	buildGameLight(pDirectCtx);
	buildGeometry(pDirectCtx);
	loadTextures(pDirectCtx);
	buildMaterials();
	buildRenderItem(pDirectCtx);
	_pSobelFilter = std::make_unique<d3d::SobelFilter>(pDirectCtx, _width, _height);
}

void Shape::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	updatePassCB(pGameTimer);
}

void Shape::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
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
		renderShapesPass(pDirectCtx);
		renderSkullPass(pDirectCtx);
		_pSobelFilter->apply(pDirectCtx, pRenderTargetBuffer);
		pDirectCtx->copyResource(pRenderTargetBuffer, _pSobelFilter->getOutput());
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void Shape::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	_pCamera->_aspect = float(width) / float(height);
	_pSobelFilter->onResize(pDirectCtx, width, height);
}

void Shape::buildTexturePSO(dx12lib::DirectContextProxy pDirectCtx) {
	dx12lib::RootSignatureDescHelper rootDesc(d3d::getStaticSamplers());
	rootDesc.resize(4);
	rootDesc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	rootDesc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	rootDesc[3].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);

	auto pPSO = _pDevice->createGraphicsPSO("TexturePSO");
	pPSO->setRootSignature(pRootSignature);
	pPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&ShapeVertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "PS", "ps_5_0"));
	pPSO->finalize();
	_PSOMap["TexturePSO"] = pPSO;
}

void Shape::buildColorPSO(dx12lib::DirectContextProxy pDirectCtx) {
	dx12lib::RootSignatureDescHelper rootDesc;
	rootDesc.resize(3);
	rootDesc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	rootDesc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);

	auto pPSO = _pDevice->createGraphicsPSO("ColorPSO");
	pPSO->setRootSignature(pRootSignature);
	pPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&SkullVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&SkullVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	pPSO->setInputLayout(inputLayout);
	pPSO->setVertexShader(d3d::compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0"));
	pPSO->setPixelShader(d3d::compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0"));
	pPSO->finalize();
	_PSOMap["ColorPSO"] = pPSO;
}

void Shape::buildRenderItem(dx12lib::DirectContextProxy pDirectCtx) {
	RenderItem boxItem;
	ObjectCB boxObjCb;

	// build Texture RenderItem
	constexpr const char *pTexturePSOName = "TexturePSO";
	auto &textureRenderItems = _renderItems[pTexturePSOName];

	boxObjCb.material = _materials["boxMat"];
	XMStoreFloat4x4(&boxObjCb.world, DX::XMMatrixScaling(2.f, 2.f, 2.f) * DX::XMMatrixTranslation(0.f, 0.5f, 0.f));
	boxItem._pMesh = _geometrys["box"];
	boxItem._pAlbedo = _textureMap["bricks.dds"];
	boxItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(boxObjCb);
	textureRenderItems.push_back(boxItem);

	RenderItem gridItem;
	ObjectCB gridObjCB;
	gridObjCB.material = _materials["gridMat"];
	gridObjCB.world = MathHelper::identity4x4();;
	gridItem._pMesh = _geometrys["grid"];
	gridItem._pAlbedo = _textureMap["tile.dds"];
	gridItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(gridObjCB);
	textureRenderItems.push_back(gridItem);

	for (std::size_t i = 0; i < 5; ++i) {
		RenderItem leftCylRItem;
		RenderItem rightCylRItem;
		RenderItem leftSphereRItem;
		RenderItem rightSphereRItem;

		ObjectCB leftCylObjCB;
		ObjectCB rightCylObjCB;
		ObjectCB leftSphereObjCB;
		ObjectCB rightSphereObjCB;

		DX::XMStoreFloat4x4(&leftCylObjCB.world, DX::XMMatrixTranslation(-5.f, 1.5f, -10.f + i * 5.f));
		DX::XMStoreFloat4x4(&rightCylObjCB.world, DX::XMMatrixTranslation(+5.f, 1.5f, -10.f + i * 5.f));
		DX::XMStoreFloat4x4(&leftSphereObjCB.world, DX::XMMatrixTranslation(-5.f, 3.5f, -10.f + i * 5.f));
		DX::XMStoreFloat4x4(&rightSphereObjCB.world, DX::XMMatrixTranslation(+5.f, 3.5f, -10.f + i * 5.f));

		leftCylObjCB.material = _materials["cylinderMat"];
		rightCylObjCB.material = _materials["cylinderMat"];
		leftSphereObjCB.material = _materials["sphereMat"];
		rightSphereObjCB.material = _materials["sphereMat"];

		leftCylRItem._pMesh = _geometrys["cylinder"];
		rightCylRItem._pMesh = _geometrys["cylinder"];
		leftSphereRItem._pMesh = _geometrys["sphere"];
		rightSphereRItem._pMesh = _geometrys["sphere"];

		leftCylRItem._pAlbedo = _textureMap["bricks.dds"];
		rightCylRItem._pAlbedo = _textureMap["bricks.dds"];
		leftSphereRItem._pAlbedo = _textureMap["bricks.dds"];
		rightSphereRItem._pAlbedo = _textureMap["bricks.dds"];

		leftCylRItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(leftCylObjCB);
		rightCylRItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(rightCylObjCB);
		leftSphereRItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(leftSphereObjCB);
		rightSphereRItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(rightSphereObjCB);

		textureRenderItems.push_back(leftCylRItem);
		textureRenderItems.push_back(rightCylRItem);
		textureRenderItems.push_back(leftSphereRItem);
		textureRenderItems.push_back(rightSphereRItem);
	}

	// build Color RenderItem
	constexpr const char *pColorPSOName = "ColorPSO";
	auto &colorRenderItems = _renderItems[pColorPSOName];
	RenderItem skullItem;
	ObjectCB skullObjCB;
	skullObjCB.material = _materials["skullMat"];
	XMStoreFloat4x4(&skullObjCB.world,
		DX::XMMatrixMultiply(DX::XMMatrixScaling(0.5f, 0.5f, 0.5f), DX::XMMatrixTranslation(0.f, 1.0f, 0.f)));
	skullItem._pMesh = _geometrys["skull"];
	skullItem._pObjectCB = pDirectCtx->createStructConstantBuffer<ObjectCB>(skullObjCB);
	colorRenderItems.push_back(skullItem);
}

void Shape::buildGeometry(dx12lib::DirectContextProxy pDirectCtx) {
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 0.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createSphere(0.5f, 20, 20);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);
	com::MeshData skull = gen.loadObjFile("resource/skull.obj");

	auto buildShapeMesh = [&](const com::MeshData &mesh) {
		std::vector<ShapeVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return ShapeVertex(v.position, v.normal, v.texcoord);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			sizeof(ShapeVertex) * vertices.size(),
			sizeof(ShapeVertex)
		);
		pMesh->_pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			sizeof(std::uint16_t) * indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return pMesh;
	};

	auto buildSkullMesh = [&](const com::MeshData &mesh) {
		std::vector<SkullVertex> vertices;
		std::vector<std::uint16_t> indices;
		vertices.reserve(mesh.vertices.size());
		indices.reserve(mesh.indices.size());
		std::transform(mesh.vertices.begin(), mesh.vertices.end(), std::back_inserter(vertices), [](auto &v) {
			return SkullVertex(v.position, v.normal);
		});
		std::transform(mesh.indices.begin(), mesh.indices.end(), std::back_inserter(indices), [](auto &i) {
			return static_cast<std::uint16_t>(i);
		});
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>();
		pMesh->_pVertexBuffer = pDirectCtx->createVertexBuffer(
			vertices.data(),
			sizeof(SkullVertex) * vertices.size(),
			sizeof(SkullVertex)
		);
		pMesh->_pIndexBuffer = pDirectCtx->createIndexBuffer(
			indices.data(),
			sizeof(std::uint16_t) * indices.size(),
			DXGI_FORMAT_R16_UINT
		);
		return pMesh;
	};

	_geometrys["box"] = buildShapeMesh(box);
	_geometrys["grid"] = buildShapeMesh(grid);
	_geometrys["sphere"] = buildShapeMesh(sphere);
	_geometrys["cylinder"] = buildShapeMesh(cylinder);
	_geometrys["skull"] = buildSkullMesh(skull);
}


void Shape::buildGameLight(dx12lib::DirectContextProxy pDirectCtx) {
	_pGameLightsCB = pDirectCtx->createStructConstantBuffer<d3d::LightCBType>();
	auto pGPUGameLightCB = _pGameLightsCB->map();
	pGPUGameLightCB->directLightCount = 1;
	pGPUGameLightCB->pointLightCount = 1;
	pGPUGameLightCB->spotLightCount = 0;
	pGPUGameLightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);

	pGPUGameLightCB->lights[0].initAsDirectionLight(float3(-3, 6, -3), float3(1.f));
	//pGPUGameLightCB->lights[1].initAsPointLight(float3(5, 6, 7), float3(0.1f, 1.f, 0.1f), 0.f, 30.f);
	//pGPUGameLightCB->lights[2].initAsSpotLight(
	//	float3(0, 10, 0),
	//	float3(0, -1, 0),
	//	float3(0.1f, 0.1f, 1.f),
	//	0,
	//	20,
	//	2
	//);
}

void Shape::buildMaterials() {
	d3d::Material sphereMat;
	sphereMat.diffuseAlbedo = float4(DX::Colors::White);
	sphereMat.roughness = 0.5f;
	sphereMat.metallic = 0.5f;
	_materials["sphereMat"] = sphereMat;

	d3d::Material boxMat;
	boxMat.diffuseAlbedo = float4(DX::Colors::White);
	boxMat.roughness = 1.0f;
	boxMat.metallic = 0.f;
	_materials["boxMat"] = boxMat;

	d3d::Material gridMat;
	gridMat.diffuseAlbedo = float4(DX::Colors::White);
	gridMat.roughness = 1.f;
	gridMat.metallic = 0.1f;
	_materials["gridMat"] = gridMat;

	d3d::Material cylinderMat;
	cylinderMat.diffuseAlbedo = float4(DX::Colors::White);
	cylinderMat.roughness = 1.f;
	cylinderMat.metallic = 0.f;
	_materials["cylinderMat"] = cylinderMat;

	d3d::Material skullMat;
	skullMat.diffuseAlbedo = float4(DX::Colors::Gold);
	skullMat.roughness = 0.8f;
	skullMat.metallic = 0.2f;
	_materials["skullMat"] = skullMat;
}

void Shape::loadTextures(dx12lib::DirectContextProxy pDirectCtx) {
	_textureMap["bricks.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/bricks.dds");
	_textureMap["tile.dds"] = pDirectCtx->createDDSTextureFromFile(L"resource/tile.dds");
}

void Shape::renderShapesPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "TexturePSO";
	auto pPSO = _PSOMap[passPSOName];
	pDirectCtx->setGraphicsPSO(pPSO);

	pDirectCtx->setStructuredConstantBuffer(_pPassCB, ShapeRootParameType::CBPass);
	pDirectCtx->setStructuredConstantBuffer(_pGameLightsCB, ShapeRootParameType::CBLight);

	auto psoRenderItems = _renderItems[passPSOName];
	for (auto &rItem : psoRenderItems) {
		pDirectCtx->setVertexBuffer(rItem._pMesh->_pVertexBuffer);
		pDirectCtx->setIndexBuffer(rItem._pMesh->_pIndexBuffer);
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pDirectCtx->setStructuredConstantBuffer(rItem._pObjectCB, ShapeRootParameType::CBObject);
		pDirectCtx->setShaderResourceBuffer(rItem._pAlbedo, ShapeRootParameType::SRAlbedo);
		pDirectCtx->drawIndexdInstanced(
			rItem._pMesh->_pIndexBuffer->getIndexCount(), 1, 0,
			0, 0
		);
	}
}

void Shape::renderSkullPass(dx12lib::DirectContextProxy pDirectCtx) {
	const std::string passPSOName = "ColorPSO";
	auto pPSO = _PSOMap[passPSOName];

	pDirectCtx->setGraphicsPSO(pPSO);
	pDirectCtx->setStructuredConstantBuffer(_pPassCB, ShapeRootParameType::CBPass);
	pDirectCtx->setStructuredConstantBuffer(_pGameLightsCB, ShapeRootParameType::CBLight);
	auto psoRenderItems = _renderItems[passPSOName];
	auto &rItem = psoRenderItems[0];
	pDirectCtx->setVertexBuffer(rItem._pMesh->_pVertexBuffer);
	pDirectCtx->setIndexBuffer(rItem._pMesh->_pIndexBuffer);
	pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDirectCtx->setStructuredConstantBuffer(rItem._pObjectCB, ShapeRootParameType::CBObject);
	pDirectCtx->drawIndexdInstanced(
		rItem._pMesh->_pIndexBuffer->getIndexCount(), 1, 0,
		0, 0
	);
}

void Shape::pollEvent() {
	while (auto event = _pInputSystem->mouse->getEvent())
		_pCamera->pollEvent(event);
}

void Shape::updatePassCB(std::shared_ptr<com::GameTimer> pGameTimer) {
	_pCamera->update();
	_pCamera->updatePassCB(_pPassCB);
	auto pGPUPassCB = _pPassCB->map();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pGPUPassCB->renderTargetSize = pRenderTarget->getRenderTargetSize();
	pGPUPassCB->invRenderTargetSize = pRenderTarget->getInvRenderTargetSize();
	pGPUPassCB->totalTime = pGameTimer->getTotalTime();
	pGPUPassCB->deltaTime = pGameTimer->getDeltaTime();
}
