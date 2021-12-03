#include "Shape.h"
#include "InputSystem/Mouse.h"
#include <DirectXColors.h>

D3D12_SHADER_BYTECODE ShaderByteCode::getVsByteCode() const {
	return { pVsByteCode->GetBufferPointer(), pVsByteCode->GetBufferSize() };
}

D3D12_SHADER_BYTECODE ShaderByteCode::getPsByteCode() const {
	return { pPsByteCode->GetBufferPointer(), pPsByteCode->GetBufferSize() };
}


void Shape::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pollEvent();

	currentFrameIndex_ = (currentFrameIndex_ + 1) % d3dUlti::kNumFrameResources;
	currentFrameResource_ = frameResources_[currentFrameIndex_].get();

	int currFence = currentFrameResource_->fence_;
	if (currentFrameResource_->fence_ != 0 && pFence_->GetCompletedValue() < currFence) {
		HANDLE event = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(pFence_->SetEventOnCompletion(currFence, event));
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	updatePassConstant(pGameTimer);
	updateObjectConstant();
}

void Shape::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	pCommandAlloc_->Reset();
	if (isWireframe_)
		pCommandList_->Reset(pCommandAlloc_.Get(), PSOs_["shapeGeo"].Get());
	else
		pCommandList_->Reset(pCommandAlloc_.Get(), PSOs_["shapeGeoWire"].Get());
	
	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	)));

	pCommandList_->RSSetViewports(1, &screenViewport_);
	pCommandList_->RSSetScissorRects(1, &scissorRect_);
	pCommandList_->ClearRenderTargetView(getCurrentBackBufferView(), DX::Colors::LightBlue, 1, &scissorRect_);
	pCommandList_->ClearDepthStencilView(
		getDepthStencilBufferView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f,
		0,
		1,
		&scissorRect_
	);
	pCommandList_->OMSetRenderTargets(1, RVPtr(getCurrentBackBufferView()), true, RVPtr(getDepthStencilBufferView()));
	pCommandList_->SetGraphicsRootSignature(pRootSignature_.Get());

	// set pass constant buffer
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(pCbvHeaps_->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(passCbvOffset_ + currentFrameIndex_, cbvSrvUavDescriptorSize_);
	pCommandList_->SetGraphicsRootDescriptorTable(0, handle);

	drawRenderItems();

	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdsList[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdsList);
	ThrowIfFailed(pSwapChain_->Present(0, 0));
	currentBackBufferIndex_ = (currentBackBufferIndex_ + 1) % kSwapChainCount;
	currentFrameResource_->fence_ = ++currentFence_;
	pCommandQueue_->Signal(pFence_.Get(), currentFence_);
}

void Shape::onResize(int width, int height) {
	constexpr float fov = 90.f;
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	DX::XMMATRIX projMat = DX::XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 100.f);
	DX::XMStoreFloat4x4(&proj_, projMat);
}

void Shape::pollEvent() {
	while (auto event = pInputSystem_->mouse->getEvent()) {
		POINT point = { event.x, event.y };
		switch (event.state_) {
		case com::Mouse::LPress:
			onMouseLPress(point);
			break;
		case com::Mouse::LRelease:
			onMouseLRelease(point);
			break;
		case com::Mouse::Move:
			onMouseMove(point);
			break;
		}
	}
}

void Shape::buildFrameResources() {
	for (int i = 0; i < d3dUlti::kNumFrameResources; ++i) {
		frameResources_.push_back(std::make_unique<FrameResource>(
			pDevice_.Get(),
			1,
			allRenderItems_.size()
		));
	}
}

void Shape::buildShapeGeometry() {
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 0.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createSphere(0.5f, 2);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);

	UINT vertexOffset = 0;
	UINT indexOffset = 0;

	SubmeshGeometry boxSubmesh;
	boxSubmesh.indexCount = static_cast<UINT>(box.indices.size());
	boxSubmesh.startIndexLocation = indexOffset;
	boxSubmesh.baseVertexLocation = vertexOffset;
	indexOffset += static_cast<UINT>(box.indices.size());
	vertexOffset += static_cast<UINT>(box.vertices.size());

	SubmeshGeometry gridSubmesh;
	gridSubmesh.indexCount = static_cast<UINT>(grid.indices.size());
	gridSubmesh.startIndexLocation = indexOffset;
	gridSubmesh.baseVertexLocation = vertexOffset;
	indexOffset += static_cast<UINT>(grid.indices.size());
	vertexOffset += static_cast<UINT>(grid.vertices.size());

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.indexCount = static_cast<UINT>(sphere.indices.size());
	sphereSubmesh.startIndexLocation = indexOffset;
	sphereSubmesh.baseVertexLocation = vertexOffset;
	indexOffset += static_cast<UINT>(sphere.indices.size());
	vertexOffset += static_cast<UINT>(sphere.vertices.size());

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.indexCount = static_cast<UINT>(cylinder.indices.size());
	cylinderSubmesh.startIndexLocation = indexOffset;
	cylinderSubmesh.baseVertexLocation = vertexOffset;
	indexOffset += static_cast<UINT>(cylinder.indices.size());
	vertexOffset += static_cast<UINT>(cylinder.vertices.size());

	auto totalVertexCount =
		box.vertices.size() + 
		grid.vertices.size() + 
		sphere.vertices.size() + 
		cylinder.vertices.size();

	std::vector<ShapeVertex> vertices;
	vertices.reserve(totalVertexCount);
	auto vertIter = std::back_inserter(vertices);
	std::transform(box.vertices.begin(), box.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::DarkGreen) };
	});
	std::transform(grid.vertices.begin(), grid.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::ForestGreen) };
	});
	std::transform(sphere.vertices.begin(), sphere.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::Crimson) };
	});
	std::transform(cylinder.vertices.begin(), cylinder.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::SteelBlue) };
	});
	
	auto totalIndexCount =
		box.indices.size() +
		grid.indices.size() +
		sphere.indices.size() +
		cylinder.indices.size();

	std::vector<com::uint16> indices;
	indices.reserve(totalIndexCount);
	auto idxIter = std::back_inserter(indices);
	std::transform(box.indices.begin(), box.indices.end(), idxIter, [](com::uint32 idx) {
		return static_cast<com::uint16>(idx);
	});
	std::transform(grid.indices.begin(), grid.indices.end(), idxIter, [](com::uint32 idx) {
		return static_cast<com::uint16>(idx);
	});
	std::transform(sphere.indices.begin(), sphere.indices.end(), idxIter, [](com::uint32 idx) {
		return static_cast<com::uint16>(idx);
	});
	std::transform(cylinder.indices.begin(), cylinder.indices.end(), idxIter, [](com::uint32 idx) {
		return static_cast<com::uint16>(idx);
	});

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(ShapeVertex));
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(com::uint16));
	auto pMeshGeo = std::make_unique<MeshGeometry>();
	pMeshGeo->name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &pMeshGeo->vertexBufferCPU));
	CopyMemory(pMeshGeo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &pMeshGeo->indexBufferCPU));
	CopyMemory(pMeshGeo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	pMeshGeo->vertexBufferGPU = createDefaultBuffer(
		pDevice_.Get(), 
		pCommandList_.Get(), 
		vertices.data(), 
		vbByteSize, 
		pMeshGeo->vertexBufferUploader
	);

	pMeshGeo->indexBufferGPU = createDefaultBuffer(
		pDevice_.Get(),
		pCommandList_.Get(),
		indices.data(),
		ibByteSize,
		pMeshGeo->indexBufferUploader
	);

	pMeshGeo->vertexBufferByteSize = sizeof(ShapeVertex);
	pMeshGeo->vertexBufferByteSize = vbByteSize;
	pMeshGeo->indexBufferByteSize = DXGI_FORMAT_R16_UINT;
	pMeshGeo->indexBufferByteSize = ibByteSize;

	pMeshGeo->drawArgs["box"] = boxSubmesh;
	pMeshGeo->drawArgs["grid"] = gridSubmesh;
	pMeshGeo->drawArgs["sphere"] = sphereSubmesh;
	pMeshGeo->drawArgs["cylinder"] = cylinderSubmesh;

	geometrice_[pMeshGeo->name] = std::move(pMeshGeo);
}


void Shape::buildRenderItems() {
	using namespace DX;
	auto *pGeometry = geometrice_["shapeGeo"].get();

	auto boxRItem = std::make_unique<d3dUlti::RenderItem>();
	XMStoreFloat4x4(&boxRItem->world, 
		DX::XMMatrixScaling(2.f, 2.f, 2.f) * DX::XMMatrixTranslation(0.f, 0.5f, 0.f));
	boxRItem->objCBIndex_ = 0;
	boxRItem->geometry_ = pGeometry;
	boxRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRItem->indexCount_ = boxRItem->geometry_->drawArgs["box"].indexCount;
	boxRItem->startIndexLocation_ = boxRItem->geometry_->drawArgs["box"].startIndexLocation;
	boxRItem->baseVertexLocation_ = boxRItem->geometry_->drawArgs["box"].baseVertexLocation;
	allRenderItems_.push_back(std::move(boxRItem));

	auto gridRItem = std::make_unique<d3dUlti::RenderItem>();
	gridRItem->world = MathHelper::identity4x4();
	gridRItem->objCBIndex_ = 1;
	gridRItem->geometry_ = pGeometry;
	gridRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRItem->indexCount_ = pGeometry->drawArgs["grid"].indexCount;
	gridRItem->startIndexLocation_ = pGeometry->drawArgs["grid"].startIndexLocation;
	gridRItem->baseVertexLocation_ = pGeometry->drawArgs["grid"].baseVertexLocation;
	allRenderItems_.push_back(std::move(gridRItem));

	UINT objCBIndex = 2;
	for (int i = 0; i < 5; ++i) {
		auto leftCylRItem = std::make_unique<d3dUlti::RenderItem>();
		auto rightCylRItem = std::make_unique<d3dUlti::RenderItem>();
		auto leftSphereRItem = std::make_unique<d3dUlti::RenderItem>();
		auto rightSphereRItem = std::make_unique<d3dUlti::RenderItem>();

		DX::XMMATRIX leftCylWorld = DX::XMMatrixTranslation(-5.f, 1.5f, -10.f + i * 5.f);
		DX::XMMATRIX rightCylWorld = DX::XMMatrixTranslation(+5.f, 1.5f, -10.f + i * 5.f);
		DX::XMMATRIX leftSphereWorld = DX::XMMatrixTranslation(-5.f, 3.5f, -10.f + i * 5.f);
		DX::XMMATRIX rightSphereWorld = DX::XMMatrixTranslation(+5.f, 3.5f, -10.f + i * 5.f);

		DX::XMStoreFloat4x4(&leftCylRItem->world, leftCylWorld);
		leftCylRItem->objCBIndex_ = objCBIndex++;
		leftCylRItem->geometry_ = pGeometry;
		leftCylRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftCylRItem->indexCount_ = pGeometry->drawArgs["cylinder"].indexCount;
		leftCylRItem->startIndexLocation_ = pGeometry->drawArgs["cylinder"].startIndexLocation;
		leftCylRItem->baseVertexLocation_ = pGeometry->drawArgs["cylinder"].baseVertexLocation;
		
		DX::XMStoreFloat4x4(&rightCylRItem->world, rightCylWorld);
		rightCylRItem->objCBIndex_ = objCBIndex++;
		rightCylRItem->geometry_ = pGeometry;

		rightCylRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRItem->indexCount_ = pGeometry->drawArgs["cylinder"].indexCount;
		rightCylRItem->startIndexLocation_ = pGeometry->drawArgs["cylinder"].startIndexLocation;
		rightCylRItem->baseVertexLocation_ = pGeometry->drawArgs["cylinder"].baseVertexLocation;

		DX::XMStoreFloat4x4(&leftSphereRItem->world, leftSphereWorld);
		leftSphereRItem->objCBIndex_ = objCBIndex++;
		leftSphereRItem->geometry_ = pGeometry;
		leftSphereRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRItem->indexCount_ = pGeometry->drawArgs["sphere"].indexCount;
		leftSphereRItem->startIndexLocation_ = pGeometry->drawArgs["sphere"].startIndexLocation;
		leftSphereRItem->baseVertexLocation_ = pGeometry->drawArgs["sphere"].baseVertexLocation;

		DX::XMStoreFloat4x4(&rightSphereRItem->world, rightSphereWorld);
		rightSphereRItem->objCBIndex_ = objCBIndex++;
		rightSphereRItem->geometry_ = pGeometry;
		rightSphereRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRItem->indexCount_ = pGeometry->drawArgs["sphere"].indexCount;
		rightSphereRItem->startIndexLocation_ = pGeometry->drawArgs["sphere"].startIndexLocation;
		rightSphereRItem->baseVertexLocation_ = pGeometry->drawArgs["sphere"].baseVertexLocation;

		allRenderItems_.push_back(std::move(leftCylRItem));
		allRenderItems_.push_back(std::move(rightCylRItem));
		allRenderItems_.push_back(std::move(leftSphereRItem));
		allRenderItems_.push_back(std::move(rightSphereRItem));
	}
	for (auto &pRenderItem : allRenderItems_)
		opaqueRItems_.push_back(pRenderItem.get());
}

void Shape::buildDescriptorHeaps() {
	UINT objCount = static_cast<UINT>(opaqueRItems_.size());
	UINT numDescriptors = (objCount + 1) * d3dUlti::kNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	passCbvOffset_ = objCount * d3dUlti::kNumFrameResources;
	pDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&pCbvHeaps_));
}


void Shape::buldConstantBufferViews() {
	UINT objCBByteSize = static_cast<UINT>(calcConstantBufferByteSize(sizeof(ObjectConstants)));
	UINT passCBByteSize = static_cast<UINT>(calcConstantBufferByteSize(sizeof(PassConstants)));
	UINT objCount = static_cast<UINT>(opaqueRItems_.size());

	for (int frameIdx = 0; frameIdx < d3dUlti::kNumFrameResources; ++frameIdx) {
		auto *pObjCb = frameResources_[frameIdx]->objectCB_->resource();
		for (UINT i = 0; i < objCount; ++i) {
			auto address = pObjCb->GetGPUVirtualAddress();
			address += static_cast<UINT64>(i * objCBByteSize);
			int heapIndex = frameIdx * objCount + i;
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(pCbvHeaps_->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, cbvSrvUavDescriptorSize_);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = address;
			cbvDesc.SizeInBytes = objCBByteSize;
			pDevice_->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	for (int frameIdx = 0; frameIdx < d3dUlti::kNumFrameResources; ++frameIdx) {
		auto *pPassCb = frameResources_[frameIdx]->passCB_->resource();
		auto address = pPassCb->GetGPUVirtualAddress();
		address += static_cast<UINT64>(frameIdx * passCBByteSize);

		auto heapIndex = passCbvOffset_ + frameIdx;
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(pCbvHeaps_->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, cbvSrvUavDescriptorSize_);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = address;
		cbvDesc.SizeInBytes = passCBByteSize;
		pDevice_->CreateConstantBufferView(&cbvDesc, handle);
	}
}

void Shape::buildShaderAndInputLayout() {
	inputLayout_ = {
		{ 
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ShapeVertex, position),  
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(ShapeVertex, color),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
		}
	};

	WRL::ComPtr<ID3DBlob> pVsByteCode = compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0");
	WRL::ComPtr<ID3DBlob> pPsByteCode = compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0");
	shaders_["shapeGeo"] = { pVsByteCode, pPsByteCode };
}

void Shape::buildRootSignature() {
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];
	CD3DX12_DESCRIPTOR_RANGE cbvTable[2];
	cbvTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	cbvTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	slotRootParameter[0].InitAsDescriptorTable(2, cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {
		1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	};

	WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSig,
		&errorBlob
	);

	if (FAILED(hr)) {
		OutputDebugString(static_cast<const char *>(errorBlob->GetBufferPointer()));
		ThrowIfFailed(hr);
	}

	ThrowIfFailed(pDevice_->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&pRootSignature_)
	));
}

void Shape::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	memset(&psoDesc, 0, sizeof(psoDesc));
	//psoDesc.pRootSignature
	psoDesc.VS = shaders_["shapeGeo"].getVsByteCode();
	psoDesc.PS = shaders_["shapeGeo"].getPsByteCode();
	psoDesc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT{});
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT{});
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT{});
	psoDesc.InputLayout = { inputLayout_.data(), static_cast<UINT>(inputLayout_.size()) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = backBufferFormat_;
	psoDesc.DSVFormat = depthStencilFormat_;
	psoDesc.SampleDesc = { getSampleCount(), getSampleQuality() };
	auto &pso = PSOs_["shapeGeo"];
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
}

void Shape::updateObjectConstant() {
	auto *pCurrObjCB = currentFrameResource_->objectCB_.get();
	for (auto &rItem : allRenderItems_) {
		if (rItem->numFramesDirty > 0) {
			DX::XMMATRIX world = DX::XMLoadFloat4x4(&rItem->world);
			ObjectConstants objConstant;
			DX::XMStoreFloat4x4(&objConstant.gWorld, world);
			pCurrObjCB->copyData(0, objConstant);
			--rItem->numFramesDirty;
		}
	}
}

void Shape::updatePassConstant(std::shared_ptr<com::GameTimer> pGameTimer) {
	DX::XMMATRIX view = DX::XMLoadFloat4x4(&view_);
	DX::XMMATRIX proj = DX::XMLoadFloat4x4(&proj_);

	DX::XMVECTOR det;
	det = DX::XMMatrixDeterminant(view);
	DX::XMMATRIX invView = DX::XMMatrixInverse(&det, view);

	det = DX::XMMatrixDeterminant(proj);
	DX::XMMATRIX invProj = DX::XMMatrixInverse(&det, proj);

	DX::XMMATRIX viewProj = DX::XMMatrixMultiply(view, proj);
	det = DX::XMMatrixDeterminant(viewProj);
	DX::XMMATRIX invViewProj = DX::XMMatrixInverse(&det, viewProj);
	
	DX::XMStoreFloat4x4(&mainPassCB_.gView, view);
	DX::XMStoreFloat4x4(&mainPassCB_.gInvView, invView);
	DX::XMStoreFloat4x4(&mainPassCB_.gProj, proj);
	DX::XMStoreFloat4x4(&mainPassCB_.gInvProj, invProj);
	DX::XMStoreFloat4x4(&mainPassCB_.gViewProj, viewProj);
	DX::XMStoreFloat4x4(&mainPassCB_.gInvView, invViewProj);
	mainPassCB_.gEyePos = eyePos_;
	mainPassCB_.gRenderTargetSize = float2(width_, height_);
	mainPassCB_.gInvRenderTargetSize = float2(1.f / width_, 1.f / height_);
	mainPassCB_.gNearZ = zNear;
	mainPassCB_.gFarZ = zFar;
	mainPassCB_.gTotalTime = pGameTimer->totalTime();
	mainPassCB_.gDeltaTime = pGameTimer->deltaTime();

	currentFrameResource_->passCB_->copyData(0, mainPassCB_);
}

void Shape::drawRenderItems() {
	for (auto &rItem : allRenderItems_) {
		pCommandList_->IASetVertexBuffers(0, 1, RVPtr(rItem->geometry_->getVertexBufferView()));
		pCommandList_->IASetIndexBuffer(RVPtr(rItem->geometry_->getIndexBufferView()));
		pCommandList_->IASetPrimitiveTopology(rItem->primitiveType_);
		UINT cbvIndex = currentFrameIndex_ * opaqueRItems_.size() + rItem->objCBIndex_;
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pCbvHeaps_->GetGPUDescriptorHandleForHeapStart());
		handle.Offset(cbvIndex, cbvSrvUavDescriptorSize_);

		pCommandList_->SetGraphicsRootDescriptorTable(1, handle);
		pCommandList_->DrawIndexedInstanced(
			rItem->indexCount_, 
			1, 
			rItem->startIndexLocation_, 
			rItem->baseVertexLocation_, 
			0
		);
	}
}

int main() {

}