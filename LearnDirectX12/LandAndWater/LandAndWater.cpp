#include "LandAndWater.h"
#include "GameTimer/GameTimer.h"
#include "D3D/Camera.h"
#include "D3D/D3DDescHelper.h"
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
#include "Geometry/GeometryGenerator.h"
#include <DirectXColors.h>

void WaterParame::init(const WaterParameDesc &desc) {
	_length = desc.length;
	_omega = DX::XM_2PI / desc.length;
	_speed = desc.speed * _omega;
	_amplitude = desc.amplitude;
	_direction = normalize(float3(desc.direction.x, 0.f, desc.direction.z));
	_steep = std::clamp(desc.steep, 0.f, 1.f) / (_omega * _amplitude * kMaxWaterParameCount);
}

MeshVertex::MeshVertex(const com::Vertex &vert) {
	position = vert.position;
	normal = vert.normal;
	texcoord = vert.texcoord;
}

WaterVertex::WaterVertex(const com::Vertex &vert) {
	position = vert.position;
	normal = vert.normal;
}

static float3 getHillPosition(const float3 &position) {
	float x = position.x;
	float z = position.z;
	return {
		position.x,
		0.3f * (z * std::sin(0.1f * x) + x * std::cos(0.1f * z)),
		position.z
	};
}

static float3 getHillNormal(const float3 &position) {
	float x = position.x;
	float z = position.z;
	return {
		-0.03f * z * std::cos(0.1f * x) - 0.3f * std::cos(0.1f * z),
		1.f,
		-0.3f * std::sin(0.1f * x) + 0.03f * x * std::sin(0.1f * z),
	};
}

static com::MeshData createLandMesh() {
	com::GometryGenerator gen;
	auto grid = gen.createGrid(160.f, 160.f, 50, 50);
	for (auto &vert : grid.vertices) {
		float3 position = vert.position;
		vert.position = getHillPosition(position);
		vert.normal = getHillNormal(position);
	}
	return grid;
}

LandAndWater::LandAndWater() {
	_title = "LandAndWater";
}

LandAndWater::~LandAndWater() {
}

void LandAndWater::onInitialize(dx12lib::CommandListProxy pCmdList) {
	buildCamera();
	buildConstantBuffer(pCmdList);
	buildTexturePSO(pCmdList);
	buildWaterPSO(pCmdList);
	buildClipPSO(pCmdList);
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
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pCmdList = pCmdQueue->createCommandListProxy();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pCmdList->setViewports(pRenderTarget->getViewport());
	pCmdList->setScissorRects(pRenderTarget->getScissiorRect());
	{
		dx12lib::RenderTargetTransitionBarrier barrierGuard = {
			pCmdList,
			pRenderTarget,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
		};

		auto pGPUPassCB = _pPassCB->cmap();
		const auto &color = pGPUPassCB->fogColor;
		pRenderTarget->getTexture(dx12lib::Color0)->clearColor({ color.x, color.y, color.z, color.w });
		pRenderTarget->getTexture(dx12lib::DepthStencil)->clearDepthStencil(1.f, 0);
		pCmdList->setRenderTarget(pRenderTarget);
		renderTexturePass(pCmdList);
		renderWireBoxPass(pCmdList);
		renderWaterPass(pCmdList);
	}
	pCmdQueue->executeCommandList(pCmdList);
	pCmdQueue->signal(_pSwapChain);
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


void LandAndWater::renderTexturePass(dx12lib::CommandListProxy pCmdList) {
	std::string_view passName = "TexturePSO";
	auto pPSO = _psoMap[passName.data()];
	pCmdList->setPipelineStateObject(pPSO);
	pCmdList->setStructConstantBuffer(_pPassCB, CBPass);
	pCmdList->setStructConstantBuffer(_pLightCB, CBLight);

	auto &renderItems = _renderItemMap[passName.data()];
	pCmdList->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto &rItem : renderItems) {
		pCmdList->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pCmdList->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pCmdList->setStructConstantBuffer(rItem._pConstantBuffer, CBObject);
		pCmdList->setShaderResourceView(rItem._pAlbedoMap, SRAlbedo);
		rItem._pMesh->drawIndexdInstanced(pCmdList);
	}
}

void LandAndWater::renderWaterPass(dx12lib::CommandListProxy pCmdList) {
	std::string_view passName = "WaterPSO";
	auto pPSO = _psoMap[passName.data()];
	pCmdList->setPipelineStateObject(pPSO);
	pCmdList->setStructConstantBuffer(_pPassCB, CBPass);
	pCmdList->setStructConstantBuffer(_pLightCB, CBLight);
	pCmdList->setStructConstantBuffer(_pWaterCB, CBWater);
	pCmdList->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto &renderItems = _renderItemMap[passName.data()];
	for (auto &rItem : renderItems) {
		pCmdList->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pCmdList->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pCmdList->setStructConstantBuffer(rItem._pConstantBuffer, CBObject);
		rItem._pMesh->drawIndexdInstanced(pCmdList);
	}
}

void LandAndWater::renderWireBoxPass(dx12lib::CommandListProxy pCmdList) {
	std::string_view passName = "ClipPSO";
	auto pPSO = _psoMap[passName.data()];
	pCmdList->setPipelineStateObject(pPSO);
	pCmdList->setStructConstantBuffer(_pPassCB, CBPass);
	pCmdList->setStructConstantBuffer(_pLightCB, CBLight);

	auto &renderItems = _renderItemMap[passName.data()];
	pCmdList->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto &rItem : renderItems) {
		pCmdList->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pCmdList->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pCmdList->setStructConstantBuffer(rItem._pConstantBuffer, CBObject);
		pCmdList->setShaderResourceView(rItem._pAlbedoMap, SRAlbedo);
		rItem._pMesh->drawIndexdInstanced(pCmdList);
	}
}

void LandAndWater::buildCamera() {
	d3d::CameraDesc desc = {
		float3(40, 40, 40),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		500.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(desc);
	_pCamera->_whellSensitivety = 5.f;
}

void LandAndWater::buildConstantBuffer(dx12lib::CommandListProxy pCmdList) {
	_pPassCB = pCmdList->createStructConstantBuffer<d3d::PassCBType>();
	_pLightCB = pCmdList->createStructConstantBuffer<d3d::LightCBType>();
	_pWaterCB = pCmdList->createStructConstantBuffer<WaterCBType>();

	// init pass fog constant buffer
	auto pGPUPassCB = _pPassCB->map();
	pGPUPassCB->fogColor = float4(0.7f, 0.7f, 0.7f, 1.f);
	pGPUPassCB->fogStart = 5.f;
	pGPUPassCB->fogEnd = 150.f;

	// init light constant buffer
	auto pGPULightCB = _pLightCB->map();
	pGPULightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCB->directLightCount = 1;
	pGPULightCB->lights[0].initAsDirectionLight(float3(0.2, 0.8, 0.2), float3(1.f));
	auto pGPUWaterCB = _pWaterCB->map();
	WaterParameDesc wpDesc0 = {
		20.f,
		4,
		0.4f,
		float3(-3, -5, 7),
		0.6f,
	};

	// init water constant buffer
	WaterParameDesc wpDesc1 = wpDesc0;
	wpDesc1.length = 20.f;
	wpDesc1.speed = 8;
	wpDesc1.amplitude = 0.41f;
	wpDesc1.steep = 1.f;
	WaterParameDesc wpDesc2 = wpDesc0;
	wpDesc2.direction = normalize(float3(+0.5f, 0.f, +0.4f));
	WaterParameDesc wpDesc3 = wpDesc0;
	wpDesc3.direction = normalize(float3(-0.9f, 0.f, -0.3f));
	wpDesc3.length = 26.f;
	wpDesc3.speed = 10;
	wpDesc3.steep = 0.6f;
	pGPUWaterCB->waterParames[0].init(wpDesc0);
	pGPUWaterCB->waterParames[1].init(wpDesc1);
	pGPUWaterCB->waterParames[2].init(wpDesc2);
	pGPUWaterCB->waterParames[3].init(wpDesc3);
}

void LandAndWater::buildTexturePSO(dx12lib::CommandListProxy pCmdList) {
	dx12lib::RootSignatureDescHelper desc(d3d::getStaticSamplers());
	desc.resize(4);
	desc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	// cbv
	desc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	desc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	desc[3].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	// srv
	auto pRootSignature = _pDevice->createRootSignature(desc);
	auto pTexturePSO = _pDevice->createGraphicsPSO("TexturePSO");
	pTexturePSO->setRootSignature(pRootSignature);
	pTexturePSO->setVertexShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "VS", "vs_5_0"));
	pTexturePSO->setPixelShader(d3d::compileShader(L"shader/texture.hlsl", nullptr, "PS", "ps_5_0"));
	pTexturePSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&MeshVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&MeshVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&MeshVertex::texcoord, "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT),
	};
	pTexturePSO->setInputLayout(inputLayout);
	pTexturePSO->finalize();
	_psoMap["TexturePSO"] = pTexturePSO;
}

void LandAndWater::buildWaterPSO(dx12lib::CommandListProxy pCmdList) {
	dx12lib::RootSignatureDescHelper desc(d3d::getStaticSamplers());
	desc.resize(4);
	desc[0].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	desc[1].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	desc[2].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	desc[3].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);
	auto pRootSignature = _pDevice->createRootSignature(desc);
	auto pWaterPSO = _pDevice->createGraphicsPSO("WaterPSO");
	pWaterPSO->setRootSignature(pRootSignature);
	pWaterPSO->setVertexShader(d3d::compileShader(L"shader/water.hlsl", nullptr, "VS", "vs_5_0"));
	pWaterPSO->setPixelShader(d3d::compileShader(L"shader/water.hlsl", nullptr, "PS", "ps_5_0"));
	pWaterPSO->setRenderTargetFormat(
		_pSwapChain->getRenderTargetFormat(),
		_pSwapChain->getDepthStencilFormat()
	);
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&WaterVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT),
		dx12lib::VInputLayoutDescHelper(&WaterVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT),
	};
	pWaterPSO->setInputLayout(inputLayout);

	// blend enable
	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	blendDesc.RenderTarget[0] = d3d::RenderTargetBlendDescHelper(d3d::RenderTargetBlendPreset::ALPHA);
	pWaterPSO->setBlendState(blendDesc);

	pWaterPSO->finalize();
	_psoMap["WaterPSO"] = pWaterPSO;
}

void LandAndWater::buildClipPSO(dx12lib::CommandListProxy pCmdList) {
	auto pClipPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(_psoMap["TexturePSO"]->clone("ClipPSO"));
	pClipPSO->setVertexShader(d3d::compileShader(L"shader/clip.hlsl", nullptr, "VS", "vs_5_0"));
	pClipPSO->setPixelShader(d3d::compileShader(L"shader/clip.hlsl", nullptr, "PS", "ps_5_0"));

	// disable cull
	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	pClipPSO->setRasterizerState(rasterizerDesc);

	pClipPSO->finalize();
	_psoMap["ClipPSO"] = pClipPSO;
}

void LandAndWater::buildGeometrys(dx12lib::CommandListProxy pCmdList) {
	com::GometryGenerator gen;
	_geometryMap["boxGeo"] = d3d::MakeMeshHelper<MeshVertex>::build(
		pCmdList,
		gen.createBox(1.5f, 1.5f, 1.5f, 3),
		"boxGeo"
	);
	_geometryMap["gridGeo"] = d3d::MakeMeshHelper<WaterVertex>::build(
		pCmdList, 
		gen.createGrid(160.f, 160.f, 50, 50),
		"gridGeo"
	);
	_geometryMap["landGeo"] = d3d::MakeMeshHelper<MeshVertex>::build(
		pCmdList,
		createLandMesh(),
		"landGeo"
	);
}

void LandAndWater::loadTextures(dx12lib::CommandListProxy pCmdList) {
	_textureMap["grass.dds"] = pCmdList->createDDSTextureFromFile(L"resources/grass.dds");
	_textureMap["WoodCrate02.dds"] = pCmdList->createDDSTextureFromFile(L"resources/WoodCrate02.dds");
	_textureMap["WireFence.dds"] = pCmdList->createDDSTextureFromFile(L"resources/WireFence.dds");
}

void LandAndWater::buildMaterials() {
	d3d::Material landMat = {
		float4(DX::Colors::White),
		1.0f,
		0.0f,
	};
	d3d::Material waterMat = {
		float4(DX::Colors::LightGreen),
		0.9f,
		0.7f,
	};
	d3d::Material boxMat = {
		float4(DX::Colors::LightBlue),
		0.5f,
		0.2f,
	};

	_materialMap["landMat"] = landMat;
	_materialMap["waterMat"] = waterMat;
	_materialMap["boxMat"] = boxMat;
}

void LandAndWater::buildRenderItems(dx12lib::CommandListProxy pCmdList) {
	auto pBoxMesh = _geometryMap["boxGeo"];
	auto boxMat = _materialMap["boxMat"];
	CBObjectType boxOBjectCB;
	boxOBjectCB.material = boxMat;
	auto boxWorldMat = DX::XMMatrixMultiply(
		DX::XMMatrixScaling(5.f, 5.f, 5.f),
		DX::XMMatrixTranslation(0.f, 5.f, 0.f)
	);
	DX::XMStoreFloat4x4(&boxOBjectCB.world, boxWorldMat);
	boxOBjectCB.normalMat = MathHelper::identity4x4();
	boxOBjectCB.matTransfrom = MathHelper::identity4x4();
	RenderItem boxRItem;
	boxRItem._pMesh = pBoxMesh;
	boxRItem._pConstantBuffer = pCmdList->createStructConstantBuffer<CBObjectType>(boxOBjectCB);
	boxRItem._pAlbedoMap = _textureMap["WireFence.dds"];

	auto pLandMesh = _geometryMap["landGeo"];
	auto landMat = _materialMap["landMat"];
	CBObjectType landOBjectCB;
	landOBjectCB.material = landMat;
	auto landMatTransfrom = DX::XMMatrixScaling(5.f, 5.f, 1.f);
	DX::XMStoreFloat4x4(&landOBjectCB.matTransfrom, landMatTransfrom);
	landOBjectCB.world = MathHelper::identity4x4();
	landOBjectCB.normalMat = MathHelper::identity4x4();
	RenderItem landRItem;
	landRItem._pMesh = pLandMesh;
	landRItem._pConstantBuffer = pCmdList->createStructConstantBuffer<CBObjectType>(landOBjectCB);
	landRItem._pAlbedoMap = _textureMap["grass.dds"];

	auto pWaterMesh = _geometryMap["gridGeo"];
	auto waterMat = _materialMap["waterMat"];
	CBObjectType waterOBjectCB;
	waterOBjectCB.material = waterMat;
	waterOBjectCB.world = MathHelper::identity4x4();
	waterOBjectCB.normalMat = MathHelper::identity4x4();
	waterOBjectCB.matTransfrom = MathHelper::identity4x4();
	RenderItem waterRItem;
	waterRItem._pMesh = pWaterMesh;
	waterRItem._pConstantBuffer = pCmdList->createStructConstantBuffer<CBObjectType>(waterOBjectCB);

	_renderItemMap["ClipPSO"].push_back(boxRItem);
	_renderItemMap["TexturePSO"].push_back(landRItem);
	_renderItemMap["WaterPSO"].push_back(waterRItem);
}

