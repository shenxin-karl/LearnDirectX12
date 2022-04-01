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
#include "D3D/d3dutil.h"
#include "Math/MathHelper.h"
#include "InputSystem/mouse.h"

BoxApp::BoxApp() {
	_title = "BoxApp";
}

void BoxApp::onInitialize(dx12lib::DirectContextProxy pDirectContext) {
	d3d::CameraDesc cameraDesc;
	cameraDesc.lookAt = float3(0, 0, 0);
	cameraDesc.lookFrom = float3(2, 2, 2);
	cameraDesc.lookUp = float3(0, 1, 0);
	cameraDesc.nearClip = 0.1f;
	cameraDesc.farClip = 100.f;
	cameraDesc.fov = 45.f;
	cameraDesc.aspect = float(_width) / float(_height);

	_pCamera = std::make_unique<d3d::CoronaCamera>(cameraDesc);
	_pMVPConstantBuffer = pDirectContext->createStructuredConstantBuffer<WVMConstantBuffer>();

	// initialize root signature
	dx12lib::RootParameter rootParame0;
	rootParame0.initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	dx12lib::RootSignatureDescHelper desc;
	desc.addRootParameter(rootParame0);
	auto pRootSignature = _pDevice->createRootSignature(desc);

	// initialize graphics pipeline state object
	_pGraphicsPSO = _pDevice->createGraphicsPSO("colorPSO");
	_pGraphicsPSO->setVertexShader(d3d::compileShader(L"shader/Color.hlsl", nullptr, "VS", "vs_5_0"));
	_pGraphicsPSO->setPixelShader(d3d::compileShader(L"shader/Color.hlsl", nullptr, "PS", "ps_5_0"));
	_pGraphicsPSO->setRootSignature(pRootSignature);
	_pGraphicsPSO->setRenderTargetFormats(
		1, RVPtr(_pSwapChain->getRenderTargetFormat()),
		_pSwapChain->getDepthStencilFormat()
	);

	// input layout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		dx12lib::VInputLayoutDescHelper(&Vertex::position, "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0),
		dx12lib::VInputLayoutDescHelper(&Vertex::color, "COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0),
	};
	_pGraphicsPSO->setInputLayout(inputLayout);
	_pGraphicsPSO->finalize();

	buildBoxGeometry(pDirectContext);
}

void BoxApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();
	auto pGPUWVM = _pMVPConstantBuffer->map();
	_pCamera->update();
	pGPUWVM->gWorldViewProj = _pCamera->getViewProj();
}

void BoxApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pDirectProxy = pCmdQueue->createDirectContextProxy();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pDirectProxy->setViewports(pRenderTarget->getViewport());
	pDirectProxy->setScissorRects(pRenderTarget->getScissiorRect());
	{
		dx12lib::RenderTargetTransitionBarrier barrierGuard = {
			pDirectProxy,
			pRenderTarget,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
		};

		float cosine = std::cos(pGameTimer->getTotalTime());
		float sine = std::sin(pGameTimer->getTotalTime());
		auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(dx12lib::Color0);
		auto pDepthStencilBuffer = pRenderTarget->getDepthStencilBuffer();
		pDirectProxy->clearColor(pRenderTargetBuffer, float4{
			cosine * 0.5f + 0.5f,
			sine * 0.5f + 0.5f,
			0.6f,
			1.f
			});
		pDirectProxy->clearDepthStencil(pDepthStencilBuffer, 1.f, 0);
		pDirectProxy->setRenderTarget(pRenderTarget);
		renderBoxPass(pDirectProxy);
	}
	pCmdQueue->executeCommandList(pDirectProxy);
	pCmdQueue->signal(_pSwapChain);
}

void BoxApp::pollEvent() {
	while (auto event = _pInputSystem->mouse->getEvent())
		_pCamera->pollEvent(event);
}

void BoxApp::buildBoxGeometry(dx12lib::DirectContextProxy pDirectContext) {
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
	_pBoxMesh->_pVertexBuffer = pDirectContext->createVertexBuffer(vertices.data(),
		sizeof(Vertex) * vertices.size(), 
		sizeof(Vertex)
	);
	_pBoxMesh->_pIndexBuffer = pDirectContext->createIndexBuffer(indices.data(),
		sizeof(std::uint16_t) * indices.size(),
		DXGI_FORMAT_R16_UINT
	);
	_pBoxMesh->_baseVertexLocation = 0;
	_pBoxMesh->_startIndexLocation = 0;
}

void BoxApp::renderBoxPass(dx12lib::DirectContextProxy pDirectContext) {
	pDirectContext->setGraphicsPSO(_pGraphicsPSO);
	pDirectContext->setStructuredConstantBuffer(_pMVPConstantBuffer, WorldViewProjCBuffer, 0);
	pDirectContext->setVertexBuffer(_pBoxMesh->_pVertexBuffer);
	pDirectContext->setIndexBuffer(_pBoxMesh->_pIndexBuffer);
	pDirectContext->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDirectContext->drawIndexdInstanced(
		_pBoxMesh->_pIndexBuffer->getIndexCount(),
		1,
		_pBoxMesh->_baseVertexLocation,
		_pBoxMesh->_startIndexLocation,
		0
	);
}
