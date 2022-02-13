#include "BoxApp.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/PipelineStateObject.h"
#include "dx12lib/RootSignature.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include <DirectXColors.h>
#include "dx12lib/IndexBuffer.h"
#include "dx12lib/VertexBuffer.h"
#include "D3D/d3dutil.h"

BoxApp::BoxApp() {
	_title = "BoxApp";
}

void BoxApp::onInitialize(dx12lib::CommandListProxy pCmdList) {
	_pPassConstantBuffer = pCmdList->createStructConstantBuffer<d3dUtil::PassConstants>();

	// initialize root signature
	dx12lib::RootParameter rootParame0;
	rootParame0.initAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	dx12lib::RootSignatureDescHelper desc;
	desc.addRootParameter(rootParame0);
	auto pRootSignature = _pDevice->createRootSignature(desc);

	// initialize graphics pipeline state object
	auto pGraphicsPSO = _pDevice->createGraphicsPSO("colorPSO");
	pGraphicsPSO->setVertexShader(compileShader(L"Color.hlsl", nullptr, "VS", "vs_5_0"));
	pGraphicsPSO->setPixelShader(compileShader(L"Color.hlsl", nullptr, "PS", "ps_5_0"));
	pGraphicsPSO->setRootSignature(pRootSignature);
	pGraphicsPSO->setPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pGraphicsPSO->setRenderTargetFormats(
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
	pGraphicsPSO->setInputLayout(inputLayout);
	pGraphicsPSO->finalize(_pDevice);

	buildBoxGeometry(pCmdList);
}

void BoxApp::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void BoxApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {

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

}
