#include "BasicTessellation.h"
#include "dx12lib/Context/ContextStd.h"
#include "dx12lib/Pipeline/PipelineStd.h"
#include "dx12lib/Device/DeviceStd.h"
#include "dx12lib/Texture/TextureStd.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "D3D/Tool/Camera.h"
#include "D3D/Tool/Mesh.h"
#include "D3D/Shader/ShaderCommon.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include <DirectXColors.h>

#include "D3D/dx12libHelper/RenderTarget.h"


namespace DX = DirectX;

BasicTessellationApp::BasicTessellationApp() : BaseApp() {
	_title = "BasicTessellationApp";
}

BasicTessellationApp::~BasicTessellationApp() {
}

void BasicTessellationApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::CameraDesc cameraDesc = {
		float3(20.f),
		float3(0.f, 1.f, 0.f),
		float3(0.f),
		45.f,
		0.1f,
		1000.f,
		static_cast<float>(_width) / static_cast<float>(_height)
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_mouseWheelSensitivity = 2.f;
	_pTessellationPSO = _pDevice->createGraphicsPSO("TessellationPSO");
	_pTessellationPSO->setVertexShader(d3d::compileShader(
		L"shader/Tessellation.hlsl", 
		nullptr, 
		"VS", 
		"vs_5_0")
	);
	_pTessellationPSO->setHullShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"HS",
		"hs_5_0"
	));
	_pTessellationPSO->setDomainShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"DS",
		"ds_5_0"
	));
	_pTessellationPSO->setPixelShader(d3d::compileShader(
		L"shader/Tessellation.hlsl",
		nullptr,
		"PS",
		"ps_5_0"
	));

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ dx12lib::VInputLayoutDescHelper(&PatchVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT) },
	};

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;

	dx12lib::RootSignatureDescHelper rootDesc;
	rootDesc.resize(2);
	rootDesc[CB_Object].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[CB_Pass].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);
	_pTessellationPSO->setRootSignature(pRootSignature);
	_pTessellationPSO->setRasterizerState(rasterizerDesc);
	_pTessellationPSO->setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
	_pTessellationPSO->setInputLayout(inputLayout);
	_pTessellationPSO->finalize();

	std::vector<PatchVertex> vertices = {
		{ float3(-10.0f, 0.0f, +10.0f) },
		{ float3(+10.0f, 0.0f, +10.0f) },
		{ float3(-10.0f, 0.0f, -10.0f) },
		{ float3(+10.0f, 0.0f, -10.0f) },
	};

	std::vector<uint16_t> indices = {
		0, 1, 2, 3,
	};

	auto pVertexBuffer = pDirectCtx->createVertexBuffer(
		vertices.data(), 
		vertices.size(), 
		sizeof(PatchVertex)
	);
	auto pIndexBuffer = pDirectCtx->createIndexBuffer(
		indices.data(),
		indices.size(),
		DXGI_FORMAT_R16_UINT
	);
	_pQuadMesh = std::make_shared<d3d::Mesh>(
		pVertexBuffer,
		pIndexBuffer
	);

	_pPassCB = pDirectCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pObjectCB = pDirectCtx->createFRConstantBuffer<float4x4>(MathHelper::identity4x4());
}

void BasicTessellationApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	while (auto event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);

	_pCamera->update(pGameTimer);
	auto pPassCBuffer = _pPassCB->map();
	_pCamera->updatePassCB(*pPassCBuffer);
	pPassCBuffer->totalTime = pGameTimer->getTotalTime();
	pPassCBuffer->deltaTime = pGameTimer->getDeltaTime();
	pPassCBuffer->renderTargetSize = _pSwapChain->getRenderTargetSize();
	pPassCBuffer->invRenderTargetSize = _pSwapChain->getInvRenderTargetSize();
}

void BasicTessellationApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{
		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectCtx);
		renderTarget.clear(pDirectCtx, float4(DirectX::Colors::Black));

		pDirectCtx->setGraphicsPSO(_pTessellationPSO);
		pDirectCtx->setConstantBuffer(_pObjectCB, CB_Object);
		pDirectCtx->setConstantBuffer(_pPassCB, CB_Pass);
		pDirectCtx->setVertexBuffer(_pQuadMesh->getVertexBuffer());
		pDirectCtx->setIndexBuffer(_pQuadMesh->getIndexBuffer());
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		_pQuadMesh->drawIndexdInstanced(pDirectCtx);

		renderTarget.unbind(pDirectCtx);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void BasicTessellationApp::onResize(dx12lib::DirectContextProxy pCmdList, int width, int height) {
	_pCamera->setAspect(static_cast<float>(width) / static_cast<float>(height));
}
