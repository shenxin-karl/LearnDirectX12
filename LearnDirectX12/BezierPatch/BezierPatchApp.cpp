#include "BezierPatchApp.h"
#include <DirectXColors.h>
#include "D3D/dx12libHelper/RenderTarget.h"
#include "dx12lib/Context/ContextStd.h"
#include "dx12lib/Pipeline/PipelineStd.h"
#include "dx12lib/Device/DeviceStd.h"
#include "dx12lib/Texture/TextureStd.h"
#include "dx12lib/Buffer/BufferStd.h"
#include "D3D/Tool/Mesh.h"
#include "D3D/Tool/Camera.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"


BezierPatchApp::BezierPatchApp() {
	_title = "BezierPatchApp";
}

BezierPatchApp::~BezierPatchApp() {
}

void BezierPatchApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	buildCamera();
	buildConstantBuffer(pDirectCtx);
	buildGeometry(pDirectCtx);
	buildBezierPatchPSO();
}

void BezierPatchApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	while (auto event = _pInputSystem->pMouse->getEvent())
		_pCamera->pollEvent(event);

	auto pGPUPassCB = _pPassCB->map();
	_pCamera->update(pGameTimer);
	_pCamera->updatePassCB(*pGPUPassCB);
	pGPUPassCB->renderTargetSize = _pSwapChain->getRenderTargetSize();
	pGPUPassCB->invRenderTargetSize = _pSwapChain->getInvRenderTargetSize();
	pGPUPassCB->deltaTime = pGameTimer->getDeltaTime();
	pGPUPassCB->totalTime = pGameTimer->getTotalTime();
}

void BezierPatchApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{
		d3d::RenderTarget renderTarget(_pSwapChain);
		renderTarget.bind(pDirectCtx);
		renderTarget.clear(pDirectCtx, float4(DirectX::Colors::Black));

		// draw bezier patch triangle
		pDirectCtx->setGraphicsPSO(_pBezierPatchPSO);
		pDirectCtx->setConstantBuffer(_pObjectCB, CB_Object);
		pDirectCtx->setConstantBuffer(_pPassCB, CB_Pass);
		pDirectCtx->setConstantBuffer(_pLightCB, CB_Light);
		pDirectCtx->setVertexBuffer(_pQuadMesh->getVertexBuffer());
		pDirectCtx->setIndexBuffer(_pQuadMesh->getIndexBuffer());
		pDirectCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);
		_pQuadMesh->drawIndexdInstanced(pDirectCtx);

		renderTarget.unbind(pDirectCtx);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void BezierPatchApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	_pCamera->setAspect(float(width) / float(height));
}

void BezierPatchApp::buildCamera() {
	d3d::CameraDesc cameraDesc = {
		float3(30.f),
		float3(0.f, 1.f, 0.f),
		float3(0.f),
		45.f,
		0.1f,
		500.f,
		float(_width) / float(_height),
	};
	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pCamera->_mouseWheelSensitivity = 3.f;
}

void BezierPatchApp::buildConstantBuffer(dx12lib::CommonContextProxy pCmdCtx) {
	_pPassCB = pCmdCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pLightCB = pCmdCtx->createFRConstantBuffer<d3d::CBLightType>();
	_pObjectCB = pCmdCtx->createFRConstantBuffer<CBObject>();

	auto pGPULightCB = _pLightCB->map();
	pGPULightCB->ambientLight = float4(0.1f, 0.1f, 0.1f, 1.f);
	pGPULightCB->directLightCount = 3;
	pGPULightCB->lights[0].initAsDirectionLight(-float3(0.57735f, -0.57735f, 0.57735f), float3(0.6f, 0.6f, 0.6f));
	pGPULightCB->lights[1].initAsDirectionLight(-float3(-0.57735f, -0.57735f, 0.57735f), float3(0.3f, 0.3f, 0.3f));
	pGPULightCB->lights[2].initAsDirectionLight(-float3(0.0f, -0.707f, -0.707f), float3(0.15f, 0.15f, 0.15f));

	auto pGPUObjectCb = _pObjectCB->map();
	pGPUObjectCb->gWorld = Math::MathHelper::identity4x4();
	pGPUObjectCb->gMaterial.diffuseAlbedo = float4(DirectX::Colors::LightSkyBlue);
	pGPUObjectCb->gMaterial.metallic = 0.5f;
	pGPUObjectCb->gMaterial.roughness = 0.5f;
}

void BezierPatchApp::buildGeometry(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	std::array<PatchVertex, 16> vertices = {
		// Row 0
		PatchVertex { float3(-10.0f, -10.0f, +15.0f) },
		PatchVertex { float3(-5.0f,  0.0f, +15.0f) },
		PatchVertex { float3(+5.0f,  0.0f, +15.0f) },
		PatchVertex { float3(+10.0f, 0.0f, +15.0f) },
		// Row 1
		PatchVertex { float3(-15.0f, 0.0f, +5.0f) },
		PatchVertex { float3(-5.0f,  0.0f, +5.0f) },
		PatchVertex { float3(+5.0f,  20.0f, +5.0f) },
		PatchVertex { float3(+15.0f, 0.0f, +5.0f) },
		// Row 2
		PatchVertex { float3(-15.0f, 0.0f, -5.0f) },
		PatchVertex { float3(-5.0f,  0.0f, -5.0f) },
		PatchVertex { float3(+5.0f,  0.0f, -5.0f) },
		PatchVertex { float3(+15.0f, 0.0f, -5.0f) },
		// Row 3
		PatchVertex { float3(-10.0f, 10.0f, -15.0f) },
		PatchVertex { float3(-5.0f,  0.0f, -15.0f) },
		PatchVertex { float3(+5.0f,  0.0f, -15.0f) },
		PatchVertex { float3(+25.0f, 10.0f, -15.0f) },
	};

	std::array<std::int16_t, 16> indices = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
		12, 13, 14, 15
	};

	auto pVertexBuffer = pGraphicsCtx->createVertexBuffer(
		vertices.data(), 
		vertices.size(),
		sizeof(PatchVertex)
	);
	auto pIndexBuffer = pGraphicsCtx->createIndexBuffer(
		indices.data(),
		indices.size(),
		DXGI_FORMAT_R16_UINT
	);

	_pQuadMesh = std::make_unique<d3d::Mesh>(pVertexBuffer, pIndexBuffer);
}

void BezierPatchApp::buildBezierPatchPSO() {
	_pBezierPatchPSO = _pDevice->createGraphicsPSO("BezierPatchPSO");
	_pBezierPatchPSO->setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(D3D12_DEFAULT);
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	_pBezierPatchPSO->setRasterizerState(rasterizerDesc);

	_pBezierPatchPSO->setVertexShader(d3d::compileShader(
		L"shader/BezierPatch.hlsl",
		nullptr,
		"VS",
		"vs_5_0"
	));
	_pBezierPatchPSO->setHullShader(d3d::compileShader(
		L"shader/BezierPatch.hlsl",
		nullptr,
		"HS",
		"hs_5_0"
	));
	_pBezierPatchPSO->setDomainShader(d3d::compileShader(
		L"shader/BezierPatch.hlsl",
		nullptr,
		"DS",
		"ds_5_0"
	));
	_pBezierPatchPSO->setPixelShader(d3d::compileShader(
		L"shader/BezierPatch.hlsl",
		nullptr,
		"PS",
		"ps_5_0"
	));

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&PatchVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT)
	};
	_pBezierPatchPSO->setInputLayout(inputLayout);

	dx12lib::RootSignatureDescHelper rootDesc;
	rootDesc.resize(3);
	rootDesc[CB_Object].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[CB_Pass].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	rootDesc[CB_Light].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);
	_pBezierPatchPSO->setRootSignature(pRootSignature);
	_pBezierPatchPSO->finalize();
}
