#include "Shape.h"
#include "D3D/Camera.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/RootSignature.h"

Shape::Shape() {
	_title = "Shape";
}

void Shape::onInitialize(dx12lib::CommandListProxy pCmdList) {
	d3dUtil::CameraDesc cameraDesc = {
		float3(3, 3, 3),
		float3(0, 1, 0),
		float3(0, 0, 0),
		45.f,
		0.1f,
		100.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3dUtil::CoronaCamera>(cameraDesc);
	_pPassCB = pCmdList->createStructConstantBuffer<d3dUtil::PassConstants>();
	_pGameLightsCB = pCmdList->createStructConstantBuffer<GameLights>();
}

void Shape::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void Shape::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void Shape::buildPSO(dx12lib::CommandListProxy pCmdList) {
	
}

void Shape::buildRenderItem(dx12lib::CommandListProxy pCmdList) {

}

void Shape::renderShapesPass(dx12lib::CommandListProxy pCmdList) {

}

void Shape::pollEvent() {

}
