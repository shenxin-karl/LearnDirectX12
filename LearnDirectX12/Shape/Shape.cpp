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
	auto iter = std::back_inserter(vertices);
	std::transform(box.vertices.begin(), box.vertices.end(), iter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::DarkGreen) };
	});
	std::transform(grid.vertices.begin(), grid.vertices.end(), iter, [](const com::Vertex &vert) {
		return ShapeVertex{ vert.position, float4(DX::Colors::ForestGreen) };
	});
	std::transform(sphere.vertices.begin(), sphere.vertices.end(), iter, [](const com::Vertex &vert) {
		// todo
	});
}

int main() {

}