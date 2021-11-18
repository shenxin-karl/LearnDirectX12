#include "BoxApp.h"
#include "InputSystem/window.h"
#include "InputSystem/Mouse.h"
#include <DirectXColors.h>
#include <array>
#include <iostream>


BoxApp::BoxApp() : BaseApp() {
	title_ = "BoxApp";
}

bool BoxApp::initialize() {
	if (!BaseApp::initialize())
		return false;

	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));
	buildDescriptorHeaps();
	buildConstantsBuffers();
	buildRootSignature();
	buildShaderAndInputLayout();
	buildBoxGeometry();
	buildPSO();
	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();
	return true;
}

void BoxApp::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::beginTick(pGameTimer);
	processEvent();
	updateConstantBuffer();
}

void BoxApp::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::tick(pGameTimer);
	ThrowIfFailed(pCommandAlloc_->Reset());
	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), pPSO_.Get()));
	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
			getCurrentBuffer(), 
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
	)));
	pCommandList_->RSSetViewports(1, &screenViewport_);
	pCommandList_->RSSetScissorRects(1, &scissorRect_);
	pCommandList_->ClearRenderTargetView(currentBackBufferView(), DX::Colors::LightBlue, 0, nullptr);
	pCommandList_->ClearDepthStencilView(depthStencilBufferView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);

	pCommandList_->OMSetRenderTargets(
		1, RVPtr(currentBackBufferView()), 
		true, RVPtr(depthStencilBufferView())
	);

	pCommandList_->IASetVertexBuffers(0, 1, RVPtr(pBoxGeo_->vertexBufferView()));
	pCommandList_->IASetIndexBuffer(RVPtr(pBoxGeo_->indexBufferView()));
	pCommandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap *descriptorHeaps[] = { pCbvHeap_.Get() };
	pCommandList_->SetDescriptorHeaps(1, descriptorHeaps);
	pCommandList_->SetGraphicsRootSignature(pRootSignature_.Get());
	pCommandList_->SetGraphicsRootDescriptorTable(0, pCbvHeap_->GetGPUDescriptorHandleForHeapStart());
	const auto &submesh = pBoxGeo_->drawArgs["box"];
	pCommandList_->DrawIndexedInstanced(submesh.indexCount, 1, 
		submesh.startIndexLocation, submesh.baseVertexLocation, 0);

	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	)));

	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);

	ThrowIfFailed(pSwapChain_->Present(0, 0));
	currBackBuffer_ = (currBackBuffer_ + 1) % kSwapChainCount;
	flushCommandQueue();
}

void BoxApp::onResize(int width, int height) {
	BaseApp::onResize(width, height);
	float aspect = pInputSystem_->window->aspectRatio();
	DX::XMMATRIX matrix = DX::XMMatrixPerspectiveFovLH(
		DX::XMConvertToRadians(45.f),
		aspect,
		0.1f,
		100.f
	);
	DX::XMStoreFloat4x4(&projMat_, matrix);
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

void BoxApp::processEvent() {
	while (auto eventObj = pInputSystem_->mouse->getEvent()) {
		switch (eventObj.state_) {
		case com::Mouse::Move:
			onMouseMove({ eventObj.x, eventObj.y });
			break;
		case com::Mouse::LPress:
			onMouseRPress();
			break;
		case com::Mouse::LRelease:
			onMouseRRelease();
			break;
		case com::Mouse::Wheel:
			onMouseWheel(eventObj.offset_);
		}
	}
}

void BoxApp::onMouseMove(POINT mousePosition) {
	if (isMouseLeftPressed_) {
		float dx = static_cast<float>(mousePosition.x - lastMousePos_.x);
		float dy = static_cast<float>(mousePosition.y - lastMousePos_.y);
		theta_ = std::clamp(theta_+dy, -89.f, +89.f);;
		phi_ -= dx;
	}
	lastMousePos_ = mousePosition;
}

void BoxApp::onMouseRPress() {
	isMouseLeftPressed_ = true;
}

void BoxApp::onMouseRRelease() {
	isMouseLeftPressed_ = false;
}


void BoxApp::onMouseWheel(float offset) {
	radius_ -= offset * 0.2f;
}

void BoxApp::updateConstantBuffer() const {
	DX::XMFLOAT3 worldUp = { 0, 1, 0 };
	DX::XMFLOAT3 lookAt = { 0, 0, 0 };
	float cosTheta = std::cos(DX::XMConvertToRadians(theta_));
	float sinTheta = std::sin(DX::XMConvertToRadians(theta_));
	float cosPhi = std::cos(DX::XMConvertToRadians(phi_));
	float sinPhi = std::sin(DX::XMConvertToRadians(phi_));
	DX::XMFLOAT3 lookFrom = {
		cosTheta * cosPhi * radius_,
		sinTheta * radius_,
		cosTheta * sinPhi * radius_
	};
	DX::XMMATRIX view = DX::XMMatrixLookAtLH(toVector3(lookFrom), toVector3(lookAt), toVector3(worldUp));
	DX::XMMATRIX proj = DX::XMLoadFloat4x4(&projMat_);
	DX::XMMATRIX world = DX::XMLoadFloat4x4(&worldMat_);
	DX::XMMATRIX worldViewProj = world * view * proj;
	ObjectConstants constantBuffer;
	DX::XMStoreFloat4x4(&constantBuffer.worldViewProj, worldViewProj);
	pObjectCB_->copyData(0, constantBuffer);
}

DX::XMVECTOR BoxApp::toVector3(const DX::XMFLOAT3 &float3) noexcept {
	return DX::XMLoadFloat3(&float3);
}
