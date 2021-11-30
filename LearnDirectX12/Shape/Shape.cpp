#include "Shape.h"
#include <DirectXColors.h>

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
	//passCbvOffset = objCount * 
}

int main() {

}