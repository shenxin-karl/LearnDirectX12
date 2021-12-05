#include "LangAndWater.h"
#include "Geometry/GeometryGenerator.h"
#include "GameTimer/GameTimer.h"

void LangAndWater::buildFrameResource() {
	UINT objectCount = static_cast<UINT>(allRenderItem_.size());
	for (size_t i = 0; i < d3dUlti::kNumFrameResources; ++i) {
		auto frameResource = std::make_unique<FrameResource>(
			pDevice_.Get(),
			1, 
			objectCount
		);
	}
}

void LangAndWater::buildLandGeometry() {
	com::GometryGenerator gen;
	auto grid = gen.createGrid(160.f, 160.f, 50, 50);

	std::vector<Vertex> vertices;
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
		vertices.push_back(Vertex{ position, color });
	}

	std::vector<com::uint16> indices;
	indices.reserve(grid.indices.size());
	for (com::uint32 i : grid.indices)
		indices.push_back(static_cast<com::uint16>(i));

	UINT vbByteSize = sizeof(Vertex) * vertices.size();
	UINT ibByteSize = sizeof(com::uint16) * indices.size();

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

	pGeo->vertexByteStride = sizeof(Vertex);
	pGeo->vertexBufferByteSize = vbByteSize;
	pGeo->indexBufferFormat = DXGI_FORMAT_R16_UINT;
	pGeo->indexBufferByteSize = ibByteSize;

	SubmeshGeometry gridSubMesh;
	gridSubMesh.baseVertexLocation = 0;
	gridSubMesh.startIndexLocation = 0;
	gridSubMesh.indexCount = indices.size();
	pGeo->drawArgs["grid"] = gridSubMesh;
	geometrices_[pGeo->name] = std::move(pGeo);
}

void LangAndWater::buildRenderItems() {
	auto &pGeo = geometrices_["landGeo"];
	auto item = std::make_unique<d3dUlti::RenderItem>();
	item->objCBIndex_ = 0;
	item->geometry_ = pGeo.get();
	item->primitiveType_ = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	item->indexCount_ = pGeo->drawArgs["grid"].indexCount;
	allRenderItem_.push_back(item);
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
	mainPassCB_.gRenderTargetSize = { width_, height_ };
	mainPassCB_.gInvRenderTargetSize = { 1.f / width_, 1.f / height_ };
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

float LangAndWater::getHillsHeight(float x, float z) {
	return 0.3f * (z * std::sin(0.1f * x) + x * std::cos(0.1f * z));
}
