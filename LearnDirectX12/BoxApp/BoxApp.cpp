#include "BoxApp.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include <DirectXColors.h>
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/VertexBuffer.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"
#include "D3D/d3dutil.h"
#include "Math/MathHelper.h"
#include "InputSystem/mouse.h"

BoxApp::BoxApp() {
	_title = "BoxApp";
}

void BoxApp::onInitialize(dx12lib::CommandListProxy pCmdList) {
	_pMVPConstantBuffer = pCmdList->createStructConstantBuffer<WVMConstantBuffer>();

	// initialize root signature
	dx12lib::RootParameter rootParame0;
	rootParame0.initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	dx12lib::RootSignatureDescHelper desc;
	desc.addRootParameter(rootParame0);
	auto pRootSignature = _pDevice->createRootSignature(desc);

	// initialize graphics pipeline state object
	_pGraphicsPSO = _pDevice->createGraphicsPSO("colorPSO");
	_pGraphicsPSO->setVertexShader(compileShader(L"shader/Color.hlsl", nullptr, "VS", "vs_5_0"));
	_pGraphicsPSO->setPixelShader(compileShader(L"shader/Color.hlsl", nullptr, "PS", "ps_5_0"));
	_pGraphicsPSO->setRootSignature(pRootSignature);
	_pGraphicsPSO->setRenderTargetFormats(
		1, RVPtr(_pSwapChain->getRenderTargetFormat()),
		_pSwapChain->getDepthStencilFormat(),
		_pDevice->getSampleCount(),
		_pDevice->getSampleQuality()
	);

	// input layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&Vertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0),
		dx12lib::VInputLayoutDescHelper(&Vertex::color, "COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0),
	};
	_pGraphicsPSO->setInputLayout(inputLayout);
	_pGraphicsPSO->finalize(_pDevice);

	buildBoxGeometry(pCmdList);
}

void BoxApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();

	auto pGPUWVM = _pMVPConstantBuffer->map();
	float phiRadians = DX::XMConvertToRadians(_phi);
	float thetaRadians = DX::XMConvertToRadians(_theta);
	float cosTh = std::cos(thetaRadians); 
	float sinTh = std::sin(thetaRadians);
	float cosPhi = std::cos(phiRadians);
	float sinPhi = std::sin(phiRadians);
	float3 direction = {
		cosPhi * cosTh,
		sinPhi,
		cosPhi * sinTh,
	};

	float aspect = float(_width) / float(_height);
	float3 lookfrom = direction * _radius;
	float3 lookat = float3(0.f, 0.f, 0.f);
	float3 lookup = float3(0.f, 1.f, 0.f);

	float4x4 world = MathHelper::identity4x4();
	DX::XMMATRIX gWorld = DX::XMLoadFloat4x4(&world);
	DX::XMMATRIX gView = DX::XMMatrixLookAtLH(lookfrom.toVec(), lookat.toVec(), lookup.toVec());
	DX::XMMATRIX gProj = DX::XMMatrixPerspectiveFovLH(DX::XMConvertToRadians(45.f), aspect, 0.1f, 100.f);
	DX::XMMATRIX gWorldViewProj = gWorld * gView * gProj;
	DX::XMStoreFloat4x4(&pGPUWVM->gWorldViewProj, gWorldViewProj);
}

void BoxApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
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

		float cosine = std::cos(pGameTimer->getTotalTime());
		float sine = std::sin(pGameTimer->getTotalTime());
		pRenderTarget->getTexture(dx12lib::Color0)->clearColor({
			cosine * 0.5f + 0.5f,
			sine * 0.5f + 0.5f,
			0.6f,
			1.f,
		});
		pRenderTarget->getTexture(dx12lib::DepthStencil)->clearDepthStencil(1.f, 0);
		pCmdList->setRenderTarget(pRenderTarget);
		renderBoxPass(pCmdList);
	}
	pCmdQueue->executeCommandList(pCmdList);
	pCmdQueue->signal(_pSwapChain);
}

void BoxApp::pollEvent() {
	while (auto event = _pInputSystem->mouse->getEvent()) {
		switch (event.state_) {
		case com::Mouse::State::LPress: {
			_isMouseLeftPress = true;
			_lastMousePosition.x = event.x;
			_lastMousePosition.y = event.y;
			break;
		}
		case com::Mouse::State::LRelease: {
			_isMouseLeftPress = false;
			break;
		}
		case com::Mouse::State::Wheel: {
			updateRadius(event.offset_);
			break;
		}
		case com::Mouse::State::Move: {
			updatePhiAndTheta(event.x, event.y);
			break;
		}
		}
	}
}

void BoxApp::updatePhiAndTheta(int x, int y) {
	if (_isMouseLeftPress) {
		constexpr float sensitivety = 0.5f;
		float dx = static_cast<float>(x - _lastMousePosition.x) * sensitivety;
		float dy = static_cast<float>(y - _lastMousePosition.y) * sensitivety;
		_phi = std::clamp(_phi + dy, -89.f, +89.f);;
		_theta -= dx;
	}
	_lastMousePosition = POINT(x, y);
}

void BoxApp::updateRadius(float offset) {
	constexpr float sensitivety = 0.1f;
	_radius = std::max(5.f, _radius - offset * sensitivety);
}

void BoxApp::buildBoxGeometry(dx12lib::CommandListProxy pCmdList) {
	std::array<Vertex, 8> vertices = {
		Vertex { float3(-1.f, -1.f, -1.f), float4(DX::Colors::White)   },
		Vertex { float3(-1.f, +1.f, -1.f), float4(DX::Colors::Black)   },
		Vertex { float3(+1.f, +1.f, -1.f), float4(DX::Colors::Red)     },
		Vertex { float3(+1.f, -1.f, -1.f), float4(DX::Colors::Green)   },
		Vertex { float3(-1.f, -1.f, +1.f), float4(DX::Colors::Blue)    },
		Vertex { float3(-1.f, +1.f, +1.f), float4(DX::Colors::Yellow)  },
		Vertex { float3(+1.f, +1.f, +1.f), float4(DX::Colors::Cyan)    },
		Vertex { float3(+1.f, -1.f, +1.f), float4(DX::Colors::Magenta) },
	};
	std::array<std::uint16_t, 36> indices = {
		0, 1, 2,	// front
		0, 2, 3,
		4, 6, 5,	// back
		4, 7, 6,
		4, 5, 1,	// left
		4, 1, 0,
		3, 2, 6,	// right
		3, 6, 7,
		1, 5, 6,	// top
		1, 6, 2,
		4, 0, 3,	// bottom
		4, 3, 7,
	};

	_pBoxMesh = std::make_unique<BoxMesh>();
	_pBoxMesh->_pVertexBuffer = pCmdList->createVertexBuffer(vertices.data(), 
		sizeof(Vertex) * vertices.size(), 
		sizeof(Vertex)
	);
	_pBoxMesh->_pIndexBuffer = pCmdList->createIndexBuffer(indices.data(),
		sizeof(std::uint16_t) * indices.size(),
		DXGI_FORMAT_R16_UINT
	);
	_pBoxMesh->_baseVertexLocation = 0;
	_pBoxMesh->_startIndexLocation = 0;
}

void BoxApp::renderBoxPass(dx12lib::CommandListProxy pCmdList) {
	pCmdList->setPipelineStateObject(_pGraphicsPSO);
	pCmdList->setStructConstantBuffer(_pMVPConstantBuffer, WorldViewProjCBuffer, 0);
	pCmdList->setVertexBuffer(_pBoxMesh->_pVertexBuffer);
	pCmdList->setIndexBuffer(_pBoxMesh->_pIndexBuffer);
	pCmdList->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->drawIndex(
		_pBoxMesh->_pIndexBuffer->getIndexCount(),
		1,
		_pBoxMesh->_baseVertexLocation,
		_pBoxMesh->_startIndexLocation,
		0
	);
}
