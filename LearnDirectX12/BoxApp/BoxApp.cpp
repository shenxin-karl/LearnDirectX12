#include "BoxApp.h"
#include <DirectXColors.h>
#include <array>


bool BoxApp::initialize() {
	if (!BaseApp::initialize())
		return false;

	buildDescriptorHeaps();
	buildConstantsBuffers();
	buildRootSignature();
	buildShaderAndInputLayout();
	buildBoxGeometry();
	buildPSO();

	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();
	return true;
}

void BoxApp::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void BoxApp::tick(std::shared_ptr<com::GameTimer> pGameTimer) {

}

void BoxApp::onResize(int width, int height) {

}

void BoxApp::buildDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(pDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&pCbvHeap_)));
}

void BoxApp::buildConstantsBuffers() {
	pObjectCB_ = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice_.Get(), 1, true);
	size_t objCBByteSize = calcConstantBufferByteSize(sizeof(ObjectConstants));
	auto address = pObjectCB_->resource()->GetGPUVirtualAddress();
	size_t boxCBufIndex = 0;
	address += boxCBufIndex * objCBByteSize;
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = address;
	cbvDesc.SizeInBytes = static_cast<UINT>(objCBByteSize);
	pDevice_->CreateConstantBufferView(
		&cbvDesc,
		pCbvHeap_->GetCPUDescriptorHandleForHeapStart()
	);
}

void BoxApp::buildRootSignature() {
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {
		1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	};

	WRL::ComPtr<ID3DBlob> serizlizedRootSig = nullptr;
	WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serizlizedRootSig,
		&errorBlob
	);
	if (FAILED(hr)) {
		OutputDebugStringA(static_cast<const char *>(errorBlob->GetBufferPointer()));
		ThrowIfFailed(hr);
	}

	ThrowIfFailed(pDevice_->CreateRootSignature(
		0,
		serizlizedRootSig->GetBufferPointer(),
		serizlizedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&pRootSignature_)
	));
}

void BoxApp::buildShaderAndInputLayout() {
	inputLayout_ = {
		{
			"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

	pVsByteCode_ = compileShader(L"shader/Color.hlsl", nullptr, "VS", "vs_5_0");
	pPsByteCode_ = compileShader(L"shader/Color.hlsl", nullptr, "PS", "ps_5_0");
	assert(pVsByteCode_);
	assert(pPsByteCode_);
}

void BoxApp::buildBoxGeometry() {
	std::array<Vertex, 8> vertices = {
		Vertex { DX::XMFLOAT3(-1.f, -1.f, -1.f), DX::XMFLOAT4(DX::Colors::White)   },
		Vertex { DX::XMFLOAT3(-1.f, +1.f, -1.f), DX::XMFLOAT4(DX::Colors::Black)   },
		Vertex { DX::XMFLOAT3(+1.f, +1.f, -1.f), DX::XMFLOAT4(DX::Colors::Red)     },
		Vertex { DX::XMFLOAT3(+1.f, -1.f, -1.f), DX::XMFLOAT4(DX::Colors::Green)   },
		Vertex { DX::XMFLOAT3(-1.f, -1.f, +1.f), DX::XMFLOAT4(DX::Colors::Blue)    },
		Vertex { DX::XMFLOAT3(-1.f, +1.f, +1.f), DX::XMFLOAT4(DX::Colors::Yellow)  },
		Vertex { DX::XMFLOAT3(+1.f, +1.f, +1.f), DX::XMFLOAT4(DX::Colors::Cyan)    },
		Vertex { DX::XMFLOAT3(+1.f, -1.f, +1.f), DX::XMFLOAT4(DX::Colors::Magenta) },
	};
	std::array<std::uint16_t, 36> indices = {
		0, 1, 2,
		0, 2, 3,
		4, 6, 5,
		4, 7, 6,
		4, 5, 1,
		4, 1, 0,
		3, 2, 6,
		1, 5, 6,
		1, 6, 2,
		4, 0, 3,
		4, 3, 7,
	};

	UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(Vertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(std::uint16_t);
	pBoxGeo_ = std::make_unique<MeshGeometry>();
	pBoxGeo_->name = "Box";
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &pBoxGeo_->vertexBufferCPU));
	memcpy(pBoxGeo_->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &pBoxGeo_->indexBufferCPU));
	memcpy(pBoxGeo_->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	pBoxGeo_->vertexBufferGPU = createDefaultBuffer(
		pDevice_.Get(), pCommandList_.Get(), vertices.data(), vbByteSize, pBoxGeo_->vertexBufferUploader
	);
	pBoxGeo_->indexBufferGPU = createDefaultBuffer(
		pDevice_.Get(), pCommandList_.Get(), indices.data(), ibByteSize, pBoxGeo_->indexBufferUploader
	);

	pBoxGeo_->vertexBufferByteSize = vbByteSize;
	pBoxGeo_->indexBufferByteSize = ibByteSize;
	pBoxGeo_->vertexByteStride = sizeof(Vertex);

	SubmeshGeometry submesh;
	submesh.baseVertexLocation = 0;
	submesh.startIndexLocation = 0;
	submesh.indexCount = static_cast<UINT>(indices.size());
	pBoxGeo_->drawArgs["box"] = submesh;
}

void BoxApp::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	memset(&psoDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.pRootSignature = pRootSignature_.Get();
	psoDesc.VS = { pVsByteCode_->GetBufferPointer(), pVsByteCode_->GetBufferSize() };
	psoDesc.PS = { pPsByteCode_->GetBufferPointer(), pPsByteCode_->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT{});
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT{});
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT{});
	psoDesc.InputLayout = { inputLayout_.data(), static_cast<UINT>(inputLayout_.size()) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = backBufferFormat_;
	psoDesc.DSVFormat = depthStencilFormat_;
	psoDesc.SampleDesc = { getSampleCount(), getSampleQuality() };
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pPSO_)));
}

void BoxApp::onMouseMove() {

}

void BoxApp::onMouseRPress() {

}

void BoxApp::onMouseRRelease() {

}
