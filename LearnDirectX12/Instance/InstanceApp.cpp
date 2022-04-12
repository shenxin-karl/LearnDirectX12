#include "InstanceApp.h"

#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/RootSignature.h"

InstanceApp::InstanceApp() {
	_title = "InstanceApp";
}

InstanceApp::~InstanceApp() {
}

void InstanceApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	buildCamera();
	buildBuffer(pDirectCtx);
	loadTextures(pDirectCtx);
	loadSkull(pDirectCtx);
	buildPSO();
	buildRenderItem();
}

void InstanceApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	
}

void InstanceApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	
}

void InstanceApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	
}

void InstanceApp::pollEvent() {

}

void InstanceApp::buildCamera() {
	d3d::CameraDesc cameraDesc = {
		float3(0.f),
		float3(0.f, 1.f, 0.f),
		float3(0.f, 0.f, 1.f),
		45.f,
		0.1f,
		1000.f,
		static_cast<float>(_width) / static_cast<float>(_height)
	};
	_pCamera = std::make_unique<d3d::FirstPersonCamera>(cameraDesc);
}

void InstanceApp::buildBuffer(dx12lib::CommandContextProxy pCommonCtx) {
	_pPassCB = pCommonCtx->createFRConstantBuffer<d3d::PassCBType>();
	_pLightCB = pCommonCtx->createConstantBuffer<d3d::LightCBType>();
	_pInstanceBuffer = pCommonCtx->createFRStructuredBuffer<InstanceData>(kMaxInstanceSize);

	auto pLight = _pLightCB->visit<d3d::LightCBType>();
	pLight->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pLight->directLightCount = 3;
	pLight->lights[0].initAsDirectionLight(float3(0.57735f, 0.57735f, 0.57735f), float3(0.6f));
	pLight->lights[1].initAsDirectionLight(float3(-0.57735f, 0.57735f, 0.57735f), float3(0.35f));
	pLight->lights[2].initAsDirectionLight(float3(0.0f, 0.707f, -0.707f), float3(0.15f));
}

void InstanceApp::loadTextures(dx12lib::CommandContextProxy pCommonCtx) {
	auto appendTexture = [&](const std::string &name, std::shared_ptr<dx12lib::ShaderResourceBuffer> pTex) {
		_textures.push_back(pTex);
		_textureIndexMap[name] = _textures.size()-1;
	};

	auto pTex1 = pCommonCtx->createDDSTextureFromFile(L"resources/grass.dds");
	auto pTex2 = pCommonCtx->createDDSTextureFromFile(L"resources/stone.dds");
	appendTexture("grass", pTex1);
	appendTexture("stone", pTex2);
}

void InstanceApp::loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	com::GometryGenerator gen;
	com::MeshData skullMesh = gen.loadObjFile("resources/skull.obj");
	d3d::MakeMeshHelper<OpaqueVertex> meshHelper;
	auto pSkullMesh = meshHelper.build(pGraphicsCtx, skullMesh);
	_geometryMap["skull"] = pSkullMesh;
}

void InstanceApp::buildPSO() {
	dx12lib::RootSignatureDescHelper rootDesc(d3d::getStaticSamplers());

}

void InstanceApp::buildRenderItem() {

}
