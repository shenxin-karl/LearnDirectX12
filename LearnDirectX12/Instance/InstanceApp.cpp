#include <random>
#include <DirectXColors.h>
#include "InstanceApp.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/ConstantBuffer.h"
#include "dx12lib/FRStructuredBuffer.hpp"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/RenderTargetBuffer.h"
#include "dx12lib/ShaderResourceBuffer.h"
#include "dx12lib/FRStructuredBuffer.hpp"
#include "dx12lib/StructuredBuffer.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/SwapChain.h"
#include "InputSystem/Keyboard.h"
#include "InputSystem/Mouse.h"
#include "GameTimer/GameTimer.h"
#include "D3D/SkyBox.h"

InstanceApp::InstanceApp() {
	_title = "InstanceApp";
}

InstanceApp::~InstanceApp() {
}

void InstanceApp::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
	d3d::SkyBoxDesc skyBoxDesc = {
		.pGraphicsCtx = pDirectCtx,
		.filename = L"resources/snowcube1024.dds",
		.renderTargetFormat = _pSwapChain->getRenderTargetFormat(),
		.depthStencilFormat = _pSwapChain->getDepthStencilFormat(),
	};
	_pSkyBox = std::make_unique<d3d::SkyBox>(skyBoxDesc);

	buildCamera();
	buildBuffer(pDirectCtx);
	loadTextures(pDirectCtx);
	loadSkull(pDirectCtx);
	buildMaterial(pDirectCtx);
	buildPSO();
	buildRenderItem();
}

void InstanceApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	auto pPass = _pPassCB->visit();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	_pCamera->update(pGameTimer);
	_pCamera->updatePassCB(*pPass);
	pPass->renderTargetSize = pRenderTarget->getRenderTargetSize();
	pPass->invRenderTargetSize = pRenderTarget->getInvRenderTargetSize();
	pPass->totalTime = pGameTimer->getTotalTime();
	pPass->deltaTime = pGameTimer->getDeltaTime();
}

void InstanceApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue();
	auto pDirectCtx = pCmdQueue->createDirectContextProxy();
	{
		auto pRenderTarget = _pSwapChain->getRenderTarget();
		auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(dx12lib::Color0);
		auto pDepthStencilBuffer = pRenderTarget->getDepthStencilBuffer();
		pDirectCtx->setViewport(pRenderTarget->getViewport());
		pDirectCtx->setScissorRect(pRenderTarget->getScissiorRect());
		pDirectCtx->setRenderTarget(pRenderTarget);
		pDirectCtx->transitionBarrier(pRenderTargetBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

		pDirectCtx->clearColor(pRenderTargetBuffer, float4(DirectX::Colors::LightSkyBlue));
		pDirectCtx->clearDepthStencil(pDepthStencilBuffer, 1.f, 0);

		pDirectCtx->setGraphicsPSO(_pInstancePSO);
		pDirectCtx->setConstantBufferView(_pPassCB->getConstantBufferView(), CB_Pass);
		pDirectCtx->setConstantBufferView(_pLightCB->getConstantBufferView(), CB_Light);
		pDirectCtx->setStructuredBuffer(_pMaterialData, SR_MaterialData);

		size_t srvCount = std::min(_textures.size(), kMaxTextureArraySize);
		for (size_t idx = 0; idx < srvCount; ++idx)
			pDirectCtx->setShaderResourceView(_textures[idx]->getShaderResourceView(), SR_DiffuseMapArray, idx);

		std::vector<RenderItem> renderItems = cullingByFrustum();
		doDrawInstance(pDirectCtx, _geometryMap["skull"], renderItems);

		_pSkyBox->render(pDirectCtx, _pCamera);

		pDirectCtx->transitionBarrier(pRenderTargetBuffer, D3D12_RESOURCE_STATE_PRESENT);
	}
	pCmdQueue->executeCommandList(pDirectCtx);
	pCmdQueue->signal(_pSwapChain);
}

void InstanceApp::onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) {
	_pCamera->setAspect(static_cast<float>(width) / static_cast<float>(height));
}

void InstanceApp::pollEvent() {
	// poll mouse event
	while (auto event = _pInputSystem->pMouse->getEvent()) {
		if (event.isLPress()) {
			_bMouseLeftPress = true;
			_pCamera->setLastMousePosition(POINT(event.x, event.y));
		} else if (event.isLRelease()) {
			_bMouseLeftPress = false;
		}

		if (_bMouseLeftPress || !event.isMove())
			_pCamera->pollEvent(event);
	}

	// poll Keyboard event
	while (auto event = _pInputSystem->pKeyboard->getKeyEvent())
		_pCamera->pollEvent(event);
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
	_pCamera->_cameraMoveSpeed = 25.f;
}

void InstanceApp::buildBuffer(dx12lib::CommonContextProxy pCommonCtx) {
	_pPassCB = pCommonCtx->createFRConstantBuffer<d3d::CBPassType>();
	_pLightCB = pCommonCtx->createConstantBuffer<d3d::CBLightType>();
	_pInstanceBuffer = pCommonCtx->createFRStructuredBuffer<InstanceData>(kMaxInstanceSize);

	auto pLight = _pLightCB->visit<d3d::CBLightType>();
	pLight->ambientLight = float4(0.2f, 0.2f, 0.2f, 1.f);
	pLight->directLightCount = 3;
	pLight->lights[0].initAsDirectionLight(float3(0.57735f, 0.57735f, 0.57735f), float3(0.6f));
	pLight->lights[1].initAsDirectionLight(float3(-0.57735f, 0.57735f, 0.57735f), float3(0.35f));
	pLight->lights[2].initAsDirectionLight(float3(0.0f, 0.707f, -0.707f), float3(0.15f));
}

void InstanceApp::loadTextures(dx12lib::CommonContextProxy pCommonCtx) {
	auto pTex1 = pCommonCtx->createDDSTexture2DFromFile(L"resources/grass.dds");
	auto pTex2 = pCommonCtx->createDDSTexture2DFromFile(L"resources/stone.dds");
	auto pTex3 = pCommonCtx->createDDSTexture2DFromFile(L"resources/white1x1.dds");
	_textures.push_back(pTex1);
	_textures.push_back(pTex2);
	_textures.push_back(pTex3);
}

void InstanceApp::loadSkull(dx12lib::GraphicsContextProxy pGraphicsCtx) {
	com::GometryGenerator gen;
	com::MeshData skullMesh = gen.loadObjFile("resources/skull.obj");
	d3d::MakeMeshHelper<OpaqueVertex> meshHelper;
	auto pSkullMesh = meshHelper.build(pGraphicsCtx, skullMesh);
	_geometryMap["skull"] = pSkullMesh;
}

void InstanceApp::buildMaterial(dx12lib::CommonContextProxy pCommonCtx) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> disInt(10, 30);
	std::uniform_real_distribution<float> disFloat(0.1f, 1.f);

	size_t count = disInt(gen);
	for (size_t idx = 0; idx < count; ++idx) {
		d3d::Material mat = {
			float4(disFloat(gen), disFloat(gen), disFloat(gen), 1.f),
			disFloat(gen),
			disFloat(gen),
		};
		_materials.push_back(mat);
	}
	_pMaterialData = pCommonCtx->createStructuredBuffer(_materials.data(), _materials.size(), sizeof(d3d::Material));
}

void InstanceApp::buildPSO() {
	UINT texArrayCount = static_cast<UINT>(std::min(kMaxTextureArraySize, _textures.size()));

	dx12lib::RootSignatureDescHelper rootDesc(d3d::getStaticSamplers());
	rootDesc.resize(5);
	rootDesc[CB_Pass].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootDesc[CB_Light].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	rootDesc[SR_InstanceData].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);
	rootDesc[SR_MaterialData].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 1);
	rootDesc[SR_DiffuseMapArray].initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, texArrayCount, 0, 0);
	auto pRootSignature = _pDevice->createRootSignature(rootDesc);

	_pInstancePSO = _pDevice->createGraphicsPSO("InstancePSO");
	_pInstancePSO->setRootSignature(pRootSignature);
	_pInstancePSO->setRenderTargetFormat(_pSwapChain->getRenderTargetFormat(), _pSwapChain->getDepthStencilFormat());

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ dx12lib::VInputLayoutDescHelper(&OpaqueVertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT) },
		{ dx12lib::VInputLayoutDescHelper(&OpaqueVertex::normal, "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT) },
	};
	_pInstancePSO->setInputLayout(inputLayout);

	std::string strTexArrayCount = std::to_string(texArrayCount);
	D3D_SHADER_MACRO pMacros[] = {
		{ "NUM", strTexArrayCount.c_str() },
		{ nullptr, nullptr },
	};
	_pInstancePSO->setVertexShader(d3d::compileShader(
		L"shader/opaqueInstance.hlsl", 
		pMacros, 
		"VS", 
		"vs_5_1"
	));
	_pInstancePSO->setPixelShader(d3d::compileShader(
		L"shader/opaqueInstance.hlsl",
		pMacros,
		"PS",
		"ps_5_1"
	));

	_pInstancePSO->finalize();
}

void InstanceApp::buildRenderItem() {
	auto pMesh = _geometryMap["skull"];
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> disMat(0, _materials.size()-1);
	std::uniform_int_distribution<size_t> disMap(0, _textures.size()-1);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;
	constexpr size_t n = 5;

	_opaqueRenderItems.resize(n * n * n);

	float x = -0.5f * width;
	float y = -0.5f * height;
	float z = -0.5f * depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (size_t k = 0; k < n; ++k) {
		for (size_t i = 0; i < n; ++i) {
			for (size_t j = 0; j < n; ++j) {
				size_t index = k * n * n + i * n + j;
				auto &item = _opaqueRenderItems[index];
				item.materialIdx = disMat(gen);
				item.diffuseMapIdx = disMap(gen);

				item.matWorld = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x+j*dx, y+i*dy, z+k*dz, 1.0f
				);
				auto matWorld = XMLoadFloat4x4(&item.matWorld);
				pMesh->getBounds().Transform(item.bounds, matWorld);
			}
		}
	}
}

std::vector<RenderItem> InstanceApp::cullingByFrustum() const {
	std::vector<RenderItem> res;
	DirectX::BoundingFrustum projSpaceFrustum = _pCamera->getViewSpaceFrustum();
	for (const auto &rItem : _opaqueRenderItems) {
		if (projSpaceFrustum.Contains(rItem.bounds) != DISJOINT)
			res.push_back(rItem);
	}
	return res;
}

void InstanceApp::doDrawInstance(dx12lib::GraphicsContextProxy pGraphicsCtx, 
	std::shared_ptr<d3d::Mesh> pMesh, 
	const std::vector<RenderItem> &renderItems)
{
	size_t idx = 0;
	std::span<InstanceData> bufferVisitor = _pInstanceBuffer->visit();
	for (const RenderItem &rItem : renderItems) {
		InstanceData &instData = bufferVisitor[idx++];
		instData.materialIdx = static_cast<uint32_t>(rItem.materialIdx);
		instData.diffuseMapIdx = static_cast<uint32_t>(rItem.diffuseMapIdx);
		instData.matWorld = rItem.matWorld;
		Matrix4 matWorld = Matrix4(instData.matWorld);
		Matrix4 invWorld = inverse(matWorld);
		Matrix4 matNormal = transpose(invWorld);
		instData.matNormal = float4x4(matNormal);
	}

	pGraphicsCtx->setStructuredBuffer(_pInstanceBuffer, SR_InstanceData);

	pGraphicsCtx->setVertexBuffer(pMesh->getVertexBuffer());
	pGraphicsCtx->setIndexBuffer(pMesh->getIndexBuffer());
	pGraphicsCtx->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pMesh->drawIndexdInstanced(pGraphicsCtx, idx);
}
