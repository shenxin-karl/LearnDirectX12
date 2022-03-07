#include "MirrorApp.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "InputSystem/InputSystem.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/window.h"
#include "GameTimer/GameTimer.h"
#include "D3D/D3DDescHelper.h"

MirrorApp::MirrorApp() {
	_title = "Mirror";
}

MirrorApp::~MirrorApp() {

}

void MirrorApp::onInitialize(dx12lib::CommandListProxy pCmdList) {
	buildCamera();
	buildConstantBuffers(pCmdList);
	loadTextures(pCmdList);
	buildMaterials();
	buildMeshs(pCmdList);
	buildPSOs();
	buildRenderItems();
}

void MirrorApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void MirrorApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void MirrorApp::onResize(dx12lib::CommandListProxy pCmdList, int width, int height) {
	_pCamera->_aspect = float(width) / float(height);
}

void MirrorApp::drawRenderItems(dx12lib::CommandListProxy pCmdList, RenderLayer layer) {
	auto pPSO = _psoMap[layer];
	pCmdList->setPipelineStateObject(pPSO);
	pCmdList->setStructConstantBuffer(_pPassCB, CBPass);
	pCmdList->setStructConstantBuffer(_pLightCB, CBLight);
	for (auto &rItem : _renderItems[layer]) {
		pCmdList->setVertexBuffer(rItem._pMesh->getVertexBuffer());
		pCmdList->setIndexBuffer(rItem._pMesh->getIndexBuffer());
		pCmdList->setStructConstantBuffer(rItem._pObjectCB, CBObject);
		pCmdList->setShaderResourceView(rItem._pAlbedoMap, SRAlbedo);
		rItem._pMesh->drawIndexdInstanced(pCmdList);
	}
}

void MirrorApp::buildCamera() {
	d3d::CameraDesc cameraDesc = {
		float3(1, 1, -1) * 20.f,
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		1000.f,
		float(_width) / float(_height)
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_whellSensitivety = 5.f;
}

void MirrorApp::buildConstantBuffers(dx12lib::CommandListProxy pCmdList) {
	_pPassCB = pCmdList->createStructConstantBuffer<d3d::PassCBType>();
	_pLightCB = pCmdList->createStructConstantBuffer<d3d::LightCBType>();
	auto pGPULightCb = _pLightCB->map();
	pGPULightCb->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCb->directLightCount = 1;
	pGPULightCb->lights[0].initAsDirectionLight(float3(3, 3, 3), float3(1.f));
}

void MirrorApp::loadTextures(dx12lib::CommandListProxy pCmdList) {
	_textureMap["checkboard.dds"] = pCmdList->createDDSTextureFromFile(L"resource/checkboard.dds");
	_textureMap["ice.dds"] = pCmdList->createDDSTextureFromFile(L"resource/ice.dds");
	_textureMap["white1x1.dds"] = pCmdList->createDDSTextureFromFile(L"resource/white1x1.dds");
	_textureMap["bricks3.dds"] = pCmdList->createDDSTextureFromFile(L"resource/bricks3.dds");
}

void MirrorApp::buildMaterials() {
	d3d::Material skullMat;
	skullMat.diffuseAlbedo = float4(DX::Colors::White);
	skullMat.roughness = 0.3f;
	skullMat.metallic = 0.5f;

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
	shadowMat.metallic = 0.001f;
	shadowMat.roughness = 0.0f;

	_materialMap["skullMat"] = skullMat;
	_materialMap["floorMat"] = floorMat;
	_materialMap["bricksMat"] = bricksMat;
	_materialMap["checkertileMat"] = checkertileMat;
	_materialMap["icemirrorMat"] = icemirrorMat;
	_materialMap["shadowMat"] = shadowMat;
}

static std::shared_ptr<d3d::Mesh> buildRoomMesh(dx12lib::CommandListProxy pCmdList) {
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

	std::array<std::int16_t, 30> indices ={
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
	auto pMesh = d3d::MakeMeshHelper<Vertex>::build(pCmdList, mesh);
	pMesh->appendSubMesh(floorSubmesh);
	pMesh->appendSubMesh(wallSubmesh);
	pMesh->appendSubMesh(mirrorSubMesh);
	return pMesh;
}

void MirrorApp::buildMeshs(dx12lib::CommandListProxy pCmdList) {
	com::GometryGenerator gen;
	auto skullMesh = gen.loadObjFile("resource/skull.obj");
	auto pSkullMesh = d3d::MakeMeshHelper<Vertex>::build(pCmdList, skullMesh, "skull");
	_meshMap["skullGeo"] = pSkullMesh;
	_meshMap["roomGeo"] = buildRoomMesh(pCmdList);
}

void MirrorApp::buildPSOs(dx12lib::CommandListProxy pCmdList) {
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
	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = TRUE;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = TRUE;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;
	mirrorDSS.FrontFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_REPLACE);
	mirrorDSS.BackFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_REPLACE);
	D3D12_BLEND_DESC mirrorBD;
	mirrorBD.RenderTarget[0].RenderTargetWriteMask = 0;
	pMirrorPSO->setBlendState(mirrorBD);
	pMirrorPSO->setDepthStencilState(mirrorDSS);
	pMirrorPSO->finalize();

	auto pReflectedPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pOpaquePSO->clone("ReflectedPSO"));
	CD3DX12_BLEND_DESC reflectedBlendDesc(D3D12_DEFAULT);
	reflectedBlendDesc.RenderTarget[0] = d3d::RenderTargetBlendDescHelper(d3d::RenderTargetBlendPreset::ALPHA);
	D3D12_DEPTH_STENCIL_DESC reflectedDDS = mirrorDSS;
	reflectedDDS.FrontFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_ZERO, D3D12_COMPARISON_FUNC_EQUAL);
	reflectedDDS.BackFace = d3d::DepthStencilOpDescHelper(d3d::DepthStendilOpPreset::SP_ZERO, D3D12_COMPARISON_FUNC_EQUAL);
	CD3DX12_RASTERIZER_DESC reflectedRasterizerDesc(D3D12_DEFAULT);
	reflectedRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	reflectedRasterizerDesc.FrontCounterClockwise = TRUE;
	pReflectedPSO->setBlendState(reflectedBlendDesc);
	pReflectedPSO->setDepthStencilState(reflectedDDS);
	pReflectedPSO->setRasterizerState(reflectedRasterizerDesc);
	pReflectedPSO->finalize();

	auto pShadowPSO = std::static_pointer_cast<dx12lib::GraphicsPSO>(pTransparentPSO->clone("ShadowPSO"));
	// todo ´ýÍêÉÆ

	_psoMap[RenderLayer::Opaque] = pOpaquePSO;
	_psoMap[RenderLayer::Mirrors] = pMirrorPSO;
	_psoMap[RenderLayer::Transparent] = pTransparentPSO;
	_psoMap[RenderLayer::Reflected] = pReflectedPSO;

}

void MirrorApp::buildRenderItems(dx12lib::CommandListProxy pCmdList) {
	RenderItem floorRItem;
	ObjectCBType floorObjectCB;
	auto pRoomGeo = _meshMap["roomGeo"];
	floorObjectCB.matWorld = Math::MathHelper::identity4x4();
	floorObjectCB.matNormal = Math::MathHelper::identity4x4();
	floorObjectCB.material = _materialMap["floorMat"];
	floorRItem._pMesh = pRoomGeo;
	floorRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(floorObjectCB);
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
	wallsRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(wallObjectCB);
	wallsRItem._submesh = pRoomGeo->getSubmesh("wall");
	_renderItems[RenderLayer::Opaque].push_back(wallsRItem);

	RenderItem skullRItem;
	ObjectCBType skullObjectCB;
	skullObjectCB.matWorld = Math::MathHelper::identity4x4();
	skullObjectCB.matNormal = Math::MathHelper::identity4x4();
	skullObjectCB.material = _materialMap["skullMat"];
	skullRItem._pMesh = _meshMap["skullGeo"];
	skullRItem._submesh = skullRItem._pMesh->getSubmesh("skull");
	skullRItem._pAlbedoMap = _textureMap["white1x1.dds"];
	skullRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(skullObjectCB);

	RenderItem reflectedSkullRItem = skullRItem;
	reflectedSkullRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(skullObjectCB);
	_renderItems[RenderLayer::Reflected].push_back(reflectedSkullRItem);

	RenderItem shadowedSkullRItem = skullRItem;
	ObjectCBType shadowSkullCB = skullObjectCB;
	shadowSkullCB.material = _materialMap["shadowMat"];
	shadowedSkullRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(shadowSkullCB);
	_renderItems[RenderLayer::Shadow].push_back(shadowedSkullRItem);

	RenderItem mirrorRItem;
	ObjectCBType mirrorObjectCB;
	mirrorObjectCB.material = _materialMap["icemirrorMat"];
	mirrorObjectCB.matWorld = Math::MathHelper::identity4x4();
	mirrorObjectCB.matNormal = Math::MathHelper::identity4x4();
	mirrorRItem._pAlbedoMap = _textureMap["ice.dds"];
	mirrorRItem._pMesh = pRoomGeo;
	mirrorRItem._pObjectCB = pCmdList->createStructConstantBuffer<ObjectCBType>(mirrorObjectCB);
	mirrorRItem._submesh = pRoomGeo->getSubmesh("mirror");
	_renderItems[RenderLayer::Mirrors].push_back(mirrorRItem);
	_renderItems[RenderLayer::Transparent].push_back(mirrorRItem);
}


