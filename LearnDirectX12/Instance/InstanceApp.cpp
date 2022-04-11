#include "InstanceApp.h"

#include "dx12lib/ConstantBuffer.h"

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
	pLight->ambientLight = float4(0.1f);
}

void InstanceApp::loadTextures(dx12lib::GraphicsContextProxy pGraphicsCtx) {

}

void InstanceApp::loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx) {

}

void InstanceApp::buildPSO() {

}

void InstanceApp::buildRenderItem() {

}
