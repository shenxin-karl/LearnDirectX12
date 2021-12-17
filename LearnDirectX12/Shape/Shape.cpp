#include "Shape.h"
#include "InputSystem/window.h"
#include "InputSystem/Mouse.h"
#include "InputSystem/Keyboard.h"
#include <DirectXColors.h>
#include <iostream>

D3D12_SHADER_BYTECODE ShaderByteCode::getVsByteCode() const {
	return { pVsByteCode->GetBufferPointer(), pVsByteCode->GetBufferSize() };
}

D3D12_SHADER_BYTECODE ShaderByteCode::getPsByteCode() const {
	return { pPsByteCode->GetBufferPointer(), pPsByteCode->GetBufferSize() };
}


bool Shape::initialize() {
	if (!BaseApp::initialize())
		return false;

	ThrowIfFailed(pCommandList_->Reset(pCommandAlloc_.Get(), nullptr));
	buildShapeGeometry();
	buildMaterials();
	buildRenderItems();
	buildFrameResources();
	buildDescriptorHeaps();
	buldConstantBufferViews();
	buildShaderAndInputLayout();
	buildRootSignature();
	buildPSO();
	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();
	return true;
}

void Shape::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::beginTick(pGameTimer);
	pollEvent();

	currentFrameIndex_ = (currentFrameIndex_ + 1) % d3dUtil::kNumFrameResources;
	currentFrameResource_ = frameResources_[currentFrameIndex_].get();

	int currFence = currentFrameResource_->fence_;
	if (currentFrameResource_->fence_ != 0 && pFence_->GetCompletedValue() < currFence) {
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(pFence_->SetEventOnCompletion(currFence, event));
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	updateViewMatrix();
	updatePassConstant(pGameTimer);
	updateObjectConstant();
	updateMaterials();
}

void Shape::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::tick(pGameTimer);
	auto &pCmdAlloc = currentFrameResource_->cmdListAlloc_;
	pCmdAlloc->Reset();
	if (isWireframe_)
		pCommandList_->Reset(pCmdAlloc.Get(), PSOs_["shapeGeoWire"].Get());
	else
		pCommandList_->Reset(pCmdAlloc.Get(), PSOs_["shapeGeo"].Get());
	
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
	ID3D12DescriptorHeap *descriptorHeaps[] = { pCbvHeaps_.Get() };
	pCommandList_->SetDescriptorHeaps(1, descriptorHeaps);

	// set pass constant buffer
	CD3DX12_GPU_DESCRIPTOR_HANDLE handle(pCbvHeaps_->GetGPUDescriptorHandleForHeapStart());
	handle.Offset(passCbvOffset_ + currentFrameIndex_, cbvSrvUavDescriptorSize_);
	pCommandList_->SetGraphicsRootDescriptorTable(d3dUtil::CB_Pass, handle);

	drawRenderItems();

	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	)));

	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdsList[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdsList);
	ThrowIfFailed(pSwapChain_->Present(0, 0));
	currentBackBufferIndex_ = (currentBackBufferIndex_ + 1) % kSwapChainCount;
	currentFrameResource_->fence_ = ++currentFence_;
	pCommandQueue_->Signal(pFence_.Get(), currentFence_);
}

void Shape::onResize(int width, int height) {
	BaseApp::onResize(width, height);
	constexpr float fov = DX::XMConvertToRadians(45.f);
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	DX::XMMATRIX projMat = DX::XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 100.f);
	DX::XMStoreFloat4x4(&proj_, projMat);
}

Shape::~Shape() {
	for (auto &pFrameResource : frameResources_) {
		auto fence = pFrameResource->fence_;
		if (fence != 0 && pFence_->GetCompletedValue() < fence) {
			HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(pFence_->SetEventOnCompletion(fence, event));
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}
	}
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
		case com::Mouse::Wheel:
			onMouseWhell(event.offset_);
			break;
		}
	}
	while (auto event = pInputSystem_->keyboard->readChar()) {
		if (event.isPressed())
			onKeyDown(event.getCharacter());
	}
}

void Shape::buildFrameResources() {
	UINT itemSize = static_cast<UINT>(allRenderItems_.size());
	UINT matSize = static_cast<UINT>(materials_.size());
	d3dUtil::FrameResourceDesc desc(1, itemSize, matSize);
	for (int i = 0; i < d3dUtil::kNumFrameResources; ++i) 
		frameResources_.push_back(std::make_unique<d3dUtil::FrameResource>(pDevice_.Get(), desc));
}

void Shape::buildShapeGeometry() {
	com::GometryGenerator gen;
	com::MeshData box = gen.createBox(1.5f, 0.5f, 1.5f, 3);
	com::MeshData grid = gen.createGrid(20.f, 30.f, 60, 40);
	com::MeshData sphere = gen.createSphere(0.5f, 2);
	com::MeshData cylinder = gen.createCylinder(0.5f, 0.3f, 3.f, 20, 20);
	com::MeshData skull = gen.loadObjFile("resource/skull.obj");

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

	SubmeshGeometry skullSubmesh;
	skullSubmesh.indexCount = static_cast<UINT>(skull.indices.size());
	skullSubmesh.startIndexLocation = indexOffset;
	skullSubmesh.baseVertexLocation = vertexOffset;
	indexOffset += static_cast<UINT>(skull.indices.size());
	vertexOffset += static_cast<UINT>(skull.vertices.size());

	auto totalVertexCount = box.vertices.size() + grid.vertices.size()
		+ sphere.vertices.size() + cylinder.vertices.size() + skull.vertices.size();

	std::vector<ShapeVertex> vertices;
	vertices.reserve(totalVertexCount);
	auto vertIter = std::back_inserter(vertices);
	std::transform(box.vertices.begin(), box.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, vert.normal };
	});
	std::transform(grid.vertices.begin(), grid.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, vert.normal };
	});
	std::transform(sphere.vertices.begin(), sphere.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, vert.normal };
	});
	std::transform(cylinder.vertices.begin(), cylinder.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, vert.normal };
	});
	std::transform(skull.vertices.begin(), skull.vertices.end(), vertIter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, vert.normal };
	});
	
	auto totalIndexCount = box.indices.size() + grid.indices.size() 
		+ sphere.indices.size() + cylinder.indices.size() + skull.indices.size();

	std::vector<com::uint32> indices;
	indices.reserve(totalIndexCount);
	auto idxIter = std::back_inserter(indices);
	std::copy(box.indices.begin(), box.indices.end(), idxIter);
	std::copy(grid.indices.begin(), grid.indices.end(), idxIter);
	std::copy(sphere.indices.begin(), sphere.indices.end(), idxIter);
	std::copy(cylinder.indices.begin(), cylinder.indices.end(), idxIter);
	std::copy(skull.indices.begin(), skull.indices.end(), idxIter);

	const UINT vbByteSize = static_cast<UINT>(vertices.size() * sizeof(ShapeVertex));
	const UINT ibByteSize = static_cast<UINT>(indices.size() * sizeof(com::uint32));
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

	pMeshGeo->vertexByteStride = sizeof(ShapeVertex);
	pMeshGeo->vertexBufferByteSize = vbByteSize;
	pMeshGeo->indexBufferFormat = DXGI_FORMAT_R32_UINT;
	pMeshGeo->indexBufferByteSize = ibByteSize;

	pMeshGeo->drawArgs["box"] = boxSubmesh;
	pMeshGeo->drawArgs["grid"] = gridSubmesh;
	pMeshGeo->drawArgs["sphere"] = sphereSubmesh;
	pMeshGeo->drawArgs["cylinder"] = cylinderSubmesh;
	pMeshGeo->drawArgs["skull"] = skullSubmesh;
	geometrice_[pMeshGeo->name] = std::move(pMeshGeo);
}


void Shape::buildRenderItems() {
	using namespace DX;
	auto *pGeometry = geometrice_["shapeGeo"].get();

	UINT objCBIndex = 0;
	auto boxRItem = std::make_unique<d3dUtil::RenderItem>();
	XMStoreFloat4x4(&boxRItem->world, 
		DX::XMMatrixScaling(2.f, 2.f, 2.f) * DX::XMMatrixTranslation(0.f, 0.5f, 0.f));
	boxRItem->objCBIndex_ = objCBIndex++;
	boxRItem->geometry_ = pGeometry;
	boxRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRItem->indexCount_ = boxRItem->geometry_->drawArgs["box"].indexCount;
	boxRItem->startIndexLocation_ = boxRItem->geometry_->drawArgs["box"].startIndexLocation;
	boxRItem->baseVertexLocation_ = boxRItem->geometry_->drawArgs["box"].baseVertexLocation;
	boxRItem->material_ = materials_["box"].get();
	allRenderItems_.push_back(std::move(boxRItem));

	auto gridRItem = std::make_unique<d3dUtil::RenderItem>();
	gridRItem->world = MathHelper::identity4x4();
	gridRItem->objCBIndex_ = objCBIndex++;
	gridRItem->geometry_ = pGeometry;
	gridRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gridRItem->indexCount_ = pGeometry->drawArgs["grid"].indexCount;
	gridRItem->startIndexLocation_ = pGeometry->drawArgs["grid"].startIndexLocation;
	gridRItem->baseVertexLocation_ = pGeometry->drawArgs["grid"].baseVertexLocation;
	gridRItem->material_ = materials_["grid"].get();
	allRenderItems_.push_back(std::move(gridRItem));

	auto skullRItem = std::make_unique<d3dUtil::RenderItem>();
	XMStoreFloat4x4(&skullRItem->world, DX::XMMatrixTranslation(0.f, 2.f, 0.f));
	skullRItem->objCBIndex_ = objCBIndex++;
	skullRItem->geometry_ = pGeometry;
	skullRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	skullRItem->indexCount_ = pGeometry->drawArgs["skull"].indexCount;
	skullRItem->startIndexLocation_ = pGeometry->drawArgs["skull"].startIndexLocation;
	skullRItem->baseVertexLocation_ = pGeometry->drawArgs["skull"].baseVertexLocation;
	skullRItem->material_ = materials_["skull"].get();
	allRenderItems_.push_back(std::move(skullRItem));

	for (int i = 0; i < 5; ++i) {
		auto leftCylRItem = std::make_unique<d3dUtil::RenderItem>();
		auto rightCylRItem = std::make_unique<d3dUtil::RenderItem>();
		auto leftSphereRItem = std::make_unique<d3dUtil::RenderItem>();
		auto rightSphereRItem = std::make_unique<d3dUtil::RenderItem>();

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
		leftCylRItem->material_ = materials_["cylinder"].get();
		
		DX::XMStoreFloat4x4(&rightCylRItem->world, rightCylWorld);
		rightCylRItem->objCBIndex_ = objCBIndex++;
		rightCylRItem->geometry_ = pGeometry;
		rightCylRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightCylRItem->indexCount_ = pGeometry->drawArgs["cylinder"].indexCount;
		rightCylRItem->startIndexLocation_ = pGeometry->drawArgs["cylinder"].startIndexLocation;
		rightCylRItem->baseVertexLocation_ = pGeometry->drawArgs["cylinder"].baseVertexLocation;
		rightCylRItem->material_ = materials_["cylinder"].get();

		DX::XMStoreFloat4x4(&leftSphereRItem->world, leftSphereWorld);
		leftSphereRItem->objCBIndex_ = objCBIndex++;
		leftSphereRItem->geometry_ = pGeometry;
		leftSphereRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		leftSphereRItem->indexCount_ = pGeometry->drawArgs["sphere"].indexCount;
		leftSphereRItem->startIndexLocation_ = pGeometry->drawArgs["sphere"].startIndexLocation;
		leftSphereRItem->baseVertexLocation_ = pGeometry->drawArgs["sphere"].baseVertexLocation;
		leftSphereRItem->material_ = materials_["sphere"].get();

		DX::XMStoreFloat4x4(&rightSphereRItem->world, rightSphereWorld);
		rightSphereRItem->objCBIndex_ = objCBIndex++;
		rightSphereRItem->geometry_ = pGeometry;
		rightSphereRItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		rightSphereRItem->indexCount_ = pGeometry->drawArgs["sphere"].indexCount;
		rightSphereRItem->startIndexLocation_ = pGeometry->drawArgs["sphere"].startIndexLocation;
		rightSphereRItem->baseVertexLocation_ = pGeometry->drawArgs["sphere"].baseVertexLocation;
		rightSphereRItem->material_ = materials_["sphere"].get();

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
	UINT numDescriptors = (objCount + 1) * d3dUtil::kNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	passCbvOffset_ = objCount * d3dUtil::kNumFrameResources;
	pDevice_->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&pCbvHeaps_));
}

void Shape::buldConstantBufferViews() {
	UINT objCBByteSize = static_cast<UINT>(calcConstantBufferByteSize(sizeof(d3dUtil::ObjectConstants)));
	UINT passCBByteSize = static_cast<UINT>(calcConstantBufferByteSize(sizeof(d3dUtil::PassConstants)));
	UINT objCount = static_cast<UINT>(opaqueRItems_.size());

	for (int frameIdx = 0; frameIdx < d3dUtil::kNumFrameResources; ++frameIdx) {
		auto *pObjCb = frameResources_[frameIdx]->objectCB_->resource();
		for (UINT i = 0; i < objCount; ++i) {
			auto address = frameResources_[frameIdx]->objectCB_->getGPUAddressByIndex(i);
			int heapIndex = frameIdx * objCount + i;
			CD3DX12_CPU_DESCRIPTOR_HANDLE handle(pCbvHeaps_->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, cbvSrvUavDescriptorSize_);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = address;
			cbvDesc.SizeInBytes = objCBByteSize;
			pDevice_->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	for (int frameIdx = 0; frameIdx < d3dUtil::kNumFrameResources; ++frameIdx) {
		auto address = frameResources_[frameIdx]->passCB_->getGPUAddressByIndex(0);
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
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(ShapeVertex, normal),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
		}
	};

	WRL::ComPtr<ID3DBlob> pVsByteCode = compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0");
	WRL::ComPtr<ID3DBlob> pPsByteCode = compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0");
	shaders_["shapeGeo"] = { pVsByteCode, pPsByteCode };
}

void Shape::buildRootSignature() {
	CD3DX12_ROOT_PARAMETER slotRootParameter[3];
	CD3DX12_DESCRIPTOR_RANGE cbvTable[2];
	cbvTable[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	cbvTable[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable[0]);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable[1]);
	slotRootParameter[2].InitAsConstantBufferView(2);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {
		3, slotRootParameter, 0, nullptr,
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

void Shape::buildMaterials() {
	int matCBIdx = 0;
	auto pSphereMat = std::make_unique<d3dUtil::Material>();
	pSphereMat->name_ = "sphere";
	pSphereMat->matCBIndex_ = matCBIdx++;
	pSphereMat->diffuseAlbedo_ = float4(DX::Colors::Red);
	pSphereMat->fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	pSphereMat->roughness_ = 0.5f;
	pSphereMat->metallic_ = 0.5f;
	materials_[pSphereMat->name_] = std::move(pSphereMat);

	auto pBoxMat = std::make_unique<d3dUtil::Material>();
	pBoxMat->name_ = "box";
	pBoxMat->matCBIndex_ = matCBIdx++;
	pBoxMat->diffuseAlbedo_ = float4(DX::Colors::Green);
	pBoxMat->fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	pBoxMat->roughness_ = 0.5f;
	pBoxMat->metallic_ = 0.5f;
	materials_[pBoxMat->name_] = std::move(pBoxMat);

	auto pGridMat = std::make_unique<d3dUtil::Material>();
	pGridMat->name_ = "grid";
	pGridMat->matCBIndex_ = matCBIdx++;
	pGridMat->diffuseAlbedo_ = float4(DX::Colors::Green);
	pGridMat->fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	pGridMat->roughness_ = 0.5f;
	pGridMat->metallic_ = 0.5f;
	materials_[pGridMat->name_] = std::move(pGridMat);

	auto pCylinderMat = std::make_unique<d3dUtil::Material>();
	pCylinderMat->name_ = "cylinder";
	pCylinderMat->matCBIndex_ = matCBIdx++;
	pCylinderMat->diffuseAlbedo_ = float4(DX::Colors::LightSkyBlue);
	pCylinderMat->fresnelR0 = float3(0.56f, 0.57f, 0.58f);
	pCylinderMat->roughness_ = 0.5f;
	pCylinderMat->metallic_ = 0.5f;
	materials_[pCylinderMat->name_] = std::move(pCylinderMat);

	auto pSkullMat = std::make_unique<d3dUtil::Material>();
	pSkullMat->name_ = "skull";
	pSkullMat->matCBIndex_ = matCBIdx++;
	pSkullMat->diffuseAlbedo_ = float4(DX::Colors::Goldenrod);
	pSkullMat->roughness_ = 0.0f;
	pSkullMat->metallic_ = 0.0f;
	materials_[pSkullMat->name_] = std::move(pSkullMat);
}

void Shape::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	memset(&psoDesc, 0, sizeof(psoDesc));
	psoDesc.pRootSignature = pRootSignature_.Get();
	psoDesc.VS = shaders_["shapeGeo"].getVsByteCode();
	psoDesc.PS = shaders_["shapeGeo"].getPsByteCode();
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
	auto &pso = PSOs_["shapeGeo"];
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

	auto wirePsoDesc = psoDesc;
	wirePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	auto &wirePSO = PSOs_["shapeGeoWire"];
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&wirePsoDesc, IID_PPV_ARGS(&wirePSO)));
}

void Shape::updateObjectConstant() {
	auto *pCurrObjCB = currentFrameResource_->objectCB_.get();
	for (auto &rItem : allRenderItems_) {
		if (rItem->numFramesDirty > 0) {
			DX::XMMATRIX world = DX::XMLoadFloat4x4(&rItem->world);
			d3dUtil::ObjectConstants objConstant;
			DX::XMStoreFloat4x4(&objConstant.gWorld, world);
			pCurrObjCB->copyData(rItem->objCBIndex_, objConstant);
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
	mainPassCB_.cbPerObjectPad1 = 0.f;
	mainPassCB_.gEyePos = eyePos_;
	mainPassCB_.gRenderTargetSize = float2(width_, height_);
	mainPassCB_.gInvRenderTargetSize = float2(1.f / width_, 1.f / height_);
	mainPassCB_.gNearZ = zNear;
	mainPassCB_.gFarZ = zFar;
	mainPassCB_.gTotalTime = pGameTimer->getTotalTime();
	mainPassCB_.gDeltaTime = pGameTimer->getDeltaTime();
	mainPassCB_.gAmbientLight = float4(0.1f, 0.1f, 0.1f, 0.0f);
	d3dUtil::Light directLight;
	directLight.direction = normalize(float3(0.3f, -0.7f, 0.1f));
	directLight.strength = float3(1.f);
	mainPassCB_.gLights[0] = directLight;
	currentFrameResource_->passCB_->copyData(0, mainPassCB_);
}

void Shape::updateMaterials() {
	for (auto &&[name, pMat] : materials_) {
		if (pMat->numFrameDirty_ > 0) {
			d3dUtil::MaterialConstants matBuffer;
			matBuffer.gDiffuseAlbedo = pMat->diffuseAlbedo_;
			matBuffer.gFresnelR0 = pMat->fresnelR0;
			matBuffer.gRoughness = pMat->roughness_;
			matBuffer.gMetallic = pMat->metallic_;
			currentFrameResource_->materialCB_->copyData(pMat->matCBIndex_, matBuffer);
		}
	}
}

void Shape::drawRenderItems() {
	for (auto &rItem : allRenderItems_) {
		pCommandList_->IASetVertexBuffers(0, 1, RVPtr(rItem->geometry_->getVertexBufferView()));
		pCommandList_->IASetIndexBuffer(RVPtr(rItem->geometry_->getIndexBufferView()));
		pCommandList_->IASetPrimitiveTopology(rItem->primitiveType_);
		UINT cbvIndex = currentFrameIndex_ * static_cast<UINT>(opaqueRItems_.size()) + rItem->objCBIndex_;
		auto handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pCbvHeaps_->GetGPUDescriptorHandleForHeapStart());
		handle.Offset(cbvIndex, cbvSrvUavDescriptorSize_);

		pCommandList_->SetGraphicsRootDescriptorTable(d3dUtil::CB_Object, handle);
		auto matAddress = currentFrameResource_->materialCB_->getGPUAddressByIndex(rItem->material_->matCBIndex_);
		pCommandList_->SetGraphicsRootConstantBufferView(d3dUtil::CBRegisterType::CB_Material, matAddress);
		pCommandList_->DrawIndexedInstanced(
			rItem->indexCount_, 
			1, 
			rItem->startIndexLocation_, 
			rItem->baseVertexLocation_, 
			0
		);
	}
}


void Shape::updateViewMatrix() {
	float cosTheta = std::cos(DX::XMConvertToRadians(theta_));
	float sinTheta = std::sin(DX::XMConvertToRadians(theta_));
	float cosPhi = std::cos(DX::XMConvertToRadians(phi_));
	float sinPhi = std::sin(DX::XMConvertToRadians(phi_));
	float3 lookfrom = {
		cosTheta * cosPhi,
		sinTheta,
		cosTheta * sinPhi,
	};
	lookfrom *= radius_;
	float3 lookat = float3(0.f);
	float3 worldUp = float3(0, 1, 0);
	DX::XMMATRIX view = DX::XMMatrixLookAtLH(lookfrom.toVec(), lookat.toVec(), worldUp.toVec());
	DX::XMStoreFloat4x4(&view_, view);
}


void Shape::onKeyDown(char key) {
	if (key == '1')
		isWireframe_ = !isWireframe_;
}


void Shape::onMouseWhell(float offset) {
	radius_ = std::clamp(radius_ - offset, 1.f, 100.f);
}

void Shape::onMouseMove(POINT point) {
	if (isLeftPressd) {
		constexpr float sensitivity = DX::XM_2PI / 360.f * 10;
		float dx = (point.x - lastMousePos_.x) * sensitivity;
		float dy = (point.y - lastMousePos_.y) * sensitivity;
		theta_ = std::clamp(theta_+dy, -89.f, +89.f);;
		phi_ -= dx;
	}
	lastMousePos_ = point;
}


void Shape::onMouseLPress(POINT point) {
	isLeftPressd = true;
}


void Shape::onMouseLRelease(POINT point) {
	isLeftPressd = false;
}

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	Shape app;
	try {
		app.initialize();
		while (!app.shouldClose()) {
			pGameTimer->newFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}