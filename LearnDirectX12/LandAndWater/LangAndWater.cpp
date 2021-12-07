#include "LangAndWater.h"
#include "Geometry/GeometryGenerator.h"
#include "GameTimer/GameTimer.h"
#include "InputSystem/Mouse.h"
#include "InputSystem/Keyboard.h"
#include <DirectXColors.h>

D3D12_SHADER_BYTECODE Shader::getVsByteCode() const {
	return { pVsByteCode->GetBufferPointer(), pVsByteCode->GetBufferSize() };
}

D3D12_SHADER_BYTECODE Shader::getPsByteCode() const {
	return { pPsByteCode->GetBufferPointer(), pPsByteCode->GetBufferSize() };
}

bool LangAndWater::initialize() {
	if (!BaseApp::initialize())
		return false;

	pCommandAlloc_->Reset();
	pCommandList_->Reset(pCommandAlloc_.Get(), nullptr);
	buildLandGeometry();
	buildWaterGeometry();
	buildRenderItems();
	buildFrameResource();
	buildShaderAndInputLayout();
	buildRootSignature();
	buildPSO();
	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);
	flushCommandQueue();
	return true;
}


void LangAndWater::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	BaseApp::beginTick(pGameTimer);

	currentFrameResourceIndex_ = (currentFrameResourceIndex_ + 1) % d3dUlti::kNumFrameResources;
	currentFrameResource_ = frameResources_[currentFrameResourceIndex_].get();
	auto fence = currentFrameResource_->fence_;
	if (fence != 0 && pFence_->GetCompletedValue() < fence) {
		HANDLE event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(pFence_->SetEventOnCompletion(fence, event));
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	updateViewMatrix();
	updatePassConstantBuffer(pGameTimer);
	updateObjectConstantBuffer();
}

void LangAndWater::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	// draw Land
	auto *pCmdAlloc = currentFrameResource_->cmdListAlloc_.Get();
	pCmdAlloc->Reset();
	ThrowIfFailed(pCommandList_->Reset(pCmdAlloc, nullptr));
	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	)));
	pCommandList_->RSSetViewports(1, &screenViewport_);
	pCommandList_->RSSetScissorRects(1, &scissorRect_);
	pCommandList_->ClearRenderTargetView(getCurrentBackBufferView(), DX::Colors::Black, 1, &scissorRect_);
	pCommandList_->ClearDepthStencilView(getDepthStencilBufferView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f, 0, 1, &scissorRect_
	);
	pCommandList_->OMSetRenderTargets(
		1, RVPtr(getCurrentBackBufferView()), 
		true, RVPtr(getDepthStencilBufferView())
	);

	pCommandList_->SetGraphicsRootSignature(pRootSignature_.Get());
	pCommandList_->SetGraphicsRootConstantBufferView(1, currentFrameResource_->passCB_->getGPUAddressByIndex(0));

	// draw Land
	pCommandList_->SetPipelineState(PSOs_["landGeo"].Get());
	drawLand();

	// draw water
	pCommandList_->SetPipelineState(PSOs_["waterGeo"].Get());
	drawWater();

	pCommandList_->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	)));
	ThrowIfFailed(pCommandList_->Close());
	ID3D12CommandList *cmdLists[] = { pCommandList_.Get() };
	pCommandQueue_->ExecuteCommandLists(1, cmdLists);

	ThrowIfFailed(pSwapChain_->Present(0, 0));
	currentBackBufferIndex_ = (currentBackBufferIndex_ + 1) % kSwapChainCount;
	currentFrameResource_->fence_ = ++currentFence_;
	pCommandQueue_->Signal(pFence_.Get(), currentFence_);
}

void LangAndWater::onResize(int width, int height) {
	BaseApp::onResize(width, height);
	constexpr float fov = DX::XMConvertToRadians(45.f);
	float aspect = static_cast<float>(width) / static_cast<float>(height);
	DX::XMMATRIX proj = DX::XMMatrixPerspectiveFovLH(fov, aspect, zNear_, zFar_);
	DX::XMStoreFloat4x4(&proj_, proj);
}

void LangAndWater::buildFrameResource() {
	UINT objectCount = static_cast<UINT>(allRenderItem_.size());
	for (size_t i = 0; i < d3dUlti::kNumFrameResources; ++i) {
		auto pFrameResource = std::make_unique<FrameResource>(
			pDevice_.Get(),
			1, 
			objectCount
		);
		frameResources_.push_back(std::move(pFrameResource));
	}
}

void LangAndWater::buildLandGeometry() {
	com::GometryGenerator gen;
	auto grid = gen.createGrid(160.f, 160.f, 50, 50);

	std::vector<LandVertex> vertices;
	vertices.reserve(grid.vertices.size());
	for (size_t i = 0; i < grid.vertices.size(); ++i) {
		const float3 &p = grid.vertices[i].position;
		float3 position = { p.x, getHillsHeight(p.x, p.z), p.z };
		float4 color;
		if (position.y < -10.f)
			color = float4(0.48f, 0.96f, 0.62f, 1.f);
		else if (position.y < 5.f)
			color = float4(0.48f, 0.77f, 0.46f, 1.f);
		else if (position.y < 12.f)
			color = float4(0.1f, 0.48f, 0.19f, 1.f);
		else if (position.y < 20.f)
			color = float4(0.45f, 0.39f, 0.34f, 1.f);
		else
			color = float4(1.f);
		vertices.push_back(LandVertex{ position, color });
	}

	std::vector<com::uint16> indices;
	indices.reserve(grid.indices.size());
	for (com::uint32 i : grid.indices)
		indices.push_back(static_cast<com::uint16>(i));

	UINT vbByteSize = sizeof(LandVertex) * static_cast<UINT>(vertices.size());
	UINT ibByteSize = sizeof(com::uint16) * static_cast<UINT>(indices.size());

	auto pGeo = std::make_unique<MeshGeometry>();
	pGeo->name = "landGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &pGeo->vertexBufferCPU));
	memcpy(pGeo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &pGeo->indexBufferCPU));
	memcpy(pGeo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	pGeo->vertexBufferGPU = createDefaultBuffer(
		pDevice_.Get(), 
		pCommandList_.Get(), 
		vertices.data(), 
		vbByteSize, 
		pGeo->vertexBufferUploader
	);
	pGeo->indexBufferGPU = createDefaultBuffer(
		pDevice_.Get(),
		pCommandList_.Get(),
		indices.data(),
		ibByteSize,
		pGeo->indexBufferUploader
	);

	pGeo->vertexByteStride = sizeof(LandVertex);
	pGeo->vertexBufferByteSize = vbByteSize;
	pGeo->indexBufferFormat = DXGI_FORMAT_R16_UINT;
	pGeo->indexBufferByteSize = ibByteSize;

	SubmeshGeometry gridSubMesh;
	gridSubMesh.baseVertexLocation = 0;
	gridSubMesh.startIndexLocation = 0;
	gridSubMesh.indexCount = static_cast<UINT>(indices.size());
	pGeo->drawArgs["landGrid"] = gridSubMesh;
	geometrices_[pGeo->name] = std::move(pGeo);
}

void LangAndWater::buildWaterGeometry() {
	com::GometryGenerator gen;
	auto grid = gen.createGrid(160.f, 160.f, 50, 50);
	std::vector<WaterVertex> vertices;
	std::vector<com::uint16> indices;
	vertices.reserve(grid.vertices.size());
	indices.reserve(grid.indices.size());
	
	for (auto &vert : grid.vertices)
		vertices.emplace_back(vert.position, vert.normal);
	for (auto index : grid.indices)
		indices.push_back(static_cast<com::uint16>(index));

	auto pGeo = std::make_unique<MeshGeometry>();
	pGeo->name = "waterGeo";
	UINT vbByteSize = static_cast<UINT>(vertices.size()) * sizeof(WaterVertex);
	UINT ibByteSize = static_cast<UINT>(indices.size()) * sizeof(com::uint16);

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &pGeo->vertexBufferCPU));
	memcpy(pGeo->vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &pGeo->indexBufferCPU));
	memcpy(pGeo->indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	pGeo->vertexBufferGPU = createDefaultBuffer(
		pDevice_.Get(),
		pCommandList_.Get(),
		vertices.data(),
		vbByteSize,
		pGeo->vertexBufferUploader
	);
	pGeo->indexBufferGPU = createDefaultBuffer(
		pDevice_.Get(),
		pCommandList_.Get(),
		indices.data(),
		ibByteSize,
		pGeo->indexBufferUploader
	);

	pGeo->vertexByteStride = sizeof(WaterVertex);
	pGeo->vertexBufferByteSize = vbByteSize;
	pGeo->indexBufferFormat = DXGI_FORMAT_R16_UINT;
	pGeo->indexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.baseVertexLocation = 0;
	submesh.startIndexLocation = 0;
	submesh.indexCount = static_cast<UINT>(indices.size());
	pGeo->drawArgs["waterGrid"] = submesh;
	geometrices_[pGeo->name] = std::move(pGeo);
}

void LangAndWater::buildRenderItems() {
	auto &pGeo = geometrices_["landGeo"];
	auto landItem = std::make_unique<d3dUlti::RenderItem>();
	landItem->objCBIndex_ = 0;
	landItem->geometry_ = pGeo.get();
	landItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	landItem->indexCount_ = pGeo->drawArgs["landGrid"].indexCount;
	opaqueItems_.push_back(landItem.get());
	allRenderItem_.push_back(std::move(landItem));
	
	auto waterItem = std::make_unique<d3dUlti::RenderItem>();
	waterItem->objCBIndex_ = 1;
	waterItem->geometry_ = pGeo.get();
	waterItem->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	waterItem->indexCount_ = pGeo->drawArgs["waterGrid"].indexCount;
	waterItems_.push_back(waterItem.get());
	allRenderItem_.push_back(std::move(waterItem));
}

void LangAndWater::buildShaderAndInputLayout() {
	inputLayout_ = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(LandVertex, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(LandVertex, color),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
		},
	};
	waterInputLayout_ = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(WaterVertex, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0,
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(WaterVertex, normal),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		},
	};

	WRL::ComPtr<ID3DBlob> pLandVsByteCode = compileShader(L"shader/color.hlsl", nullptr, "VS", "vs_5_0");
	WRL::ComPtr<ID3DBlob> pLandPsByteCode = compileShader(L"shader/color.hlsl", nullptr, "PS", "ps_5_0");
	WRL::ComPtr<ID3DBlob> pWaterVsByteCode = compileShader(L"shader/water.hlsl", nullptr, "VS", "vs_5_0");
	WRL::ComPtr<ID3DBlob> pWaterPsByteCode = compileShader(L"shader/water.hlsl", nullptr, "PS", "ps_5_0");
	shaders_["landGeo"] = { pLandVsByteCode, pLandPsByteCode };
	shaders_["waterGeo"] = { pWaterVsByteCode, pWaterPsByteCode };
}

void LangAndWater::buildRootSignature() {
	CD3DX12_ROOT_PARAMETER rootParameter[2];
	rootParameter[0].InitAsConstantBufferView(0);
	rootParameter[1].InitAsConstantBufferView(1);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {
		2, rootParameter, 0, nullptr,
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

void LangAndWater::buildPSO() {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC landPsoDesc;
	memset(&landPsoDesc, 0, sizeof(landPsoDesc));
	landPsoDesc.pRootSignature = pRootSignature_.Get();
	landPsoDesc.VS = shaders_["landGeo"].getVsByteCode();
	landPsoDesc.PS = shaders_["landGeo"].getPsByteCode();
	landPsoDesc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT{});
	landPsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT{});
	landPsoDesc.InputLayout = { inputLayout_.data(), static_cast<UINT>(inputLayout_.size()) };
	landPsoDesc.SampleMask = 0xffffffff;
	landPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT{});
	landPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	landPsoDesc.NumRenderTargets = 1;
	landPsoDesc.RTVFormats[0] = backBufferFormat_;
	landPsoDesc.DSVFormat = depthStencilFormat_;
	landPsoDesc.SampleDesc = { getSampleCount(), getSampleQuality() };
	auto &landPso = PSOs_["landGeo"];
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&landPsoDesc, IID_PPV_ARGS(&landPso)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC waterPsoDesc = landPsoDesc;
	waterPsoDesc.VS = shaders_["waterGeo"].getVsByteCode();
	waterPsoDesc.PS = shaders_["waterGeo"].getPsByteCode();
	waterPsoDesc.InputLayout = { waterInputLayout_.data(), static_cast<UINT>(waterInputLayout_.size()) };
	auto &waterPso = PSOs_["waterGeo"];
	ThrowIfFailed(pDevice_->CreateGraphicsPipelineState(&waterPsoDesc, IID_PPV_ARGS(&waterPso)));
}

void LangAndWater::updatePassConstantBuffer(std::shared_ptr<com::GameTimer> pGameTimer) {
	DX::XMVECTOR det;
	DX::XMMATRIX view = DX::XMLoadFloat4x4(&view_);
	det = DX::XMMatrixDeterminant(view);
	DX::XMMATRIX invView = DX::XMMatrixInverse(&det, view);

	DX::XMMATRIX proj = DX::XMLoadFloat4x4(&proj_);
	det = DX::XMMatrixDeterminant(proj);
	DX::XMMATRIX invProj = DX::XMMatrixInverse(&det, proj);

	DX::XMMATRIX viewProj = DX::XMMatrixMultiply(view, proj);
	det = DX::XMMatrixDeterminant(viewProj);
	DX::XMMATRIX invViewProj = DX::XMMatrixInverse(&det, viewProj);

	DX::XMStoreFloat4x4(&mainPassCB_.gView, view);
	DX::XMStoreFloat4x4(&mainPassCB_.gInvView, invView);
	DX::XMStoreFloat4x4(&mainPassCB_.gProj, proj);
	DX::XMStoreFloat4x4(&mainPassCB_.gInvProj, invProj);
	mainPassCB_.gEyePos = eyePos_;
	mainPassCB_.cbPerObjectPad1 = 0.f;
	mainPassCB_.gRenderTargetSize = float2(width_, height_);
	mainPassCB_.gInvRenderTargetSize = float2(1.f / width_, 1.f / height_);
	mainPassCB_.gNearZ = zNear_;
	mainPassCB_.gFarZ = zFar_;
	mainPassCB_.gTotalTime = pGameTimer->getTotalTime();
	mainPassCB_.gDeltaTime = pGameTimer->getDeltaTime();
	currentFrameResource_->passCB_->copyData(0, mainPassCB_);
}

void LangAndWater::updateObjectConstantBuffer() {
	for (auto &item : allRenderItem_) {
		if (item->numFramesDirty > 0) {
			DX::XMMATRIX world = DX::XMLoadFloat4x4(&item->world);
			ObjectConstants objCB;
			DX::XMStoreFloat4x4(&objCB.gWorld, world);
			currentFrameResource_->objectCB_->copyData(item->objCBIndex_, objCB);
			--item->numFramesDirty;
		}
	}
}

void LangAndWater::updateViewMatrix() {
	float sinTheta = std::sin(DX::XMConvertToRadians(theta_));
	float cosTheta = std::cos(DX::XMConvertToRadians(theta_));
	float sinPhi = std::sin(DX::XMConvertToRadians(phi_));
	float cosPhi = std::cos(DX::XMConvertToRadians(phi_));
	float3 lookat = float3(0);
	float3 lookup = float3(0, 1, 0);
	float3 lookfrom = {
		cosTheta * cosPhi,
		sinTheta,
		cosTheta * sinPhi,
	};
	lookfrom *= radius_;
	eyePos_ = lookfrom;
	DX::XMMATRIX view = DX::XMMatrixLookAtLH(lookfrom.toVec(), lookat.toVec(), lookup.toVec());
	DX::XMStoreFloat4x4(&view_, view);
}


void LangAndWater::drawLand() {
	for (auto &rItem : opaqueItems_) {
		pCommandList_->IASetVertexBuffers(0, 1, RVPtr(rItem->geometry_->getVertexBufferView()));
		pCommandList_->IASetIndexBuffer(RVPtr(rItem->geometry_->getIndexBufferView()));
		pCommandList_->IASetPrimitiveTopology(rItem->primitiveType_);
		auto address = currentFrameResource_->objectCB_->getGPUAddressByIndex(rItem->objCBIndex_);
		pCommandList_->SetGraphicsRootConstantBufferView(0, address);
		pCommandList_->DrawIndexedInstanced(
			rItem->indexCount_, 
			1, 
			rItem->startIndexLocation_, 
			rItem->baseVertexLocation_, 
			0
		);
	}
}


void LangAndWater::drawWater() {
	for (auto &ri : waterItems_) {
		pCommandList_->IASetVertexBuffers(0, 1, RVPtr(ri->geometry_->getVertexBufferView()));
		pCommandList_->IASetIndexBuffer(RVPtr(ri->geometry_->getIndexBufferView()));
		pCommandList_->IASetPrimitiveTopology(ri->primitiveType_);
		auto address = currentFrameResource_->objectCB_->getGPUAddressByIndex(ri->objCBIndex_);
		pCommandList_->SetGraphicsRootConstantBufferView(0, address);
		pCommandList_->DrawIndexedInstanced(
			ri->indexCount_,
			1,
			ri->startIndexLocation_,
			ri->baseVertexLocation_,
			0
		);
	}
}

float LangAndWater::getHillsHeight(float x, float z) {
	return 0.3f * (z * std::sin(0.1f * x) + x * std::cos(0.1f * z));
}

void LangAndWater::handleEvent() {
	while (auto event = pInputSystem_->mouse->getEvent()) {
		switch (event.state_) {
		case com::Mouse::LPress:
			onMouseLPress();
			break;
		case com::Mouse::LRelease:
			onMouseLRelease();
			break;
		case com::Mouse::Move:
			onMouseMove({ event.x, event.y });
			break;
		}
	}
	while (auto event = pInputSystem_->keyboard->readChar())
		onCharacter(event.getCharacter());
	
}

void LangAndWater::onMouseMove(POINT point) {
	if (isLeftPressed_) {
		constexpr float sensitivity = 360.f / 20.f;
		float dx = (point.x - lastMousePos_.x) * sensitivity;
		float dy = (point.y - lastMousePos_.y) * sensitivity;
		theta_ = std::clamp(theta_ + dy, -89.f, +89.f);
		phi_ -= dx;
	}
	lastMousePos_ = point;
}

void LangAndWater::onMouseLPress() {
	isLeftPressed_ = true;
}

void LangAndWater::onMouseLRelease() {
	isLeftPressed_ = false;
}

void LangAndWater::onCharacter(char character) {

}
