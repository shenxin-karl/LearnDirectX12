#include "HalfEdgeMesh.h"

namespace HalfEdge {

Mesh::Mesh(const com::MeshData &mesh) {
	for (auto &vert : mesh.vertices)
		insertVertex(vert.position, vert.texcoord);

	for (com::uint32 i = 0; i < mesh.indices.size()-2; i += 2) {
		com::uint32 idx0 = mesh.indices[i + static_cast<size_t>(0)];
		com::uint32 idx1 = mesh.indices[i + static_cast<size_t>(1)];
		com::uint32 idx2 = mesh.indices[i + static_cast<size_t>(2)];
		auto *vert0 = verts[idx0].get();
		auto *vert1 = verts[idx1].get();
		auto *vert2 = verts[idx2].get();
		auto *edge0 = insertEdge(vert0, vert1);
		auto *edge1 = insertEdge(vert1, vert2);
		auto *edge2 = insertEdge(vert2, vert0);
		faces.emplace_back(std::make_unique<Face>(vert0, vert1, vert2));
		auto *face = faces.back().get();
		edge0->face = edge1->face = edge2->face = face;
	}
}

Vertex *Mesh::insertVertex(const float3 &position, const float2 &texcoord) {
	auto ptr = std::make_unique<Vertex>();
	Vertex *result = ptr.get();
	ptr->position = position;
	ptr->texcoord = texcoord;
	ptr->index = verts.size();
	verts.push_back(std::move(ptr));
	return result;
}

}