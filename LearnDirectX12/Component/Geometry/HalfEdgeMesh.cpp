#include "HalfEdgeMesh.h"
#include <unordered_set>

namespace HalfEdge {

HEMesh::HEMesh(const com::MeshData &mesh) {
	for (auto &vert : mesh.vertices)
		insertVertex(vert.position, vert.texcoord);

	for (com::uint32 i = 0; i < mesh.indices.size() - 2; i += 2) {
		com::uint32 idx0 = mesh.indices[i + static_cast<std::size_t>(0)];
		com::uint32 idx1 = mesh.indices[i + static_cast<std::size_t>(1)];
		com::uint32	idx2 = mesh.indices[i + static_cast<std::size_t>(2)];
		insertFace({ idx0, idx1, idx2 });
	}
}

HEMesh::HEMesh(const std::vector<HEVertex> &vertices, std::vector<com::uint32> &indices) {
	for (auto &vert : vertices)
		insertVertex(vert.position, vert.texcoord);

	for (com::uint32 i = 0; i < indices.size() - 2; i += 2) {
		com::uint32 idx0 = indices[i + static_cast<std::size_t>(0)];
		com::uint32 idx1 = indices[i + static_cast<std::size_t>(1)];
		com::uint32	idx2 = indices[i + static_cast<std::size_t>(2)];
		insertFace({ idx0, idx1, idx2 });
	}
}

HEMesh::HEMesh(const HEMesh &other) {
	for (auto &vert : other.verts)
		insertVertex(vert->position, vert->texcoord);

	other.foreachFace([this](const HEFace *pFace) {
		std::vector<int> indices;
		indices.reserve(3);
		for (auto *pEdge : pFace->edges)
			indices.push_back(pEdge->start->index);
	});
}

void HEMesh::foreachFace(const std::function<void(const HEFace *)> &callback) const {
	for (auto &face : faces)
		callback(face.get());
}

std::vector<HEFace *> HEMesh::getFaceFromVertex(HEVertex *vert) const {
	std::vector<HEFace *> result;
	if (auto iter = faceMap.find(vert); iter != faceMap.end())
		return iter->second;
	return {};
}

std::vector<HEVertex *> HEMesh::getVertsFromVertex(HEVertex *vert) const {
	std::vector<HEVertex *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return {};

	for (auto *pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.push_back(pEdge->last);
			else if (pEdge->last == vert)
				result.push_back(pEdge->start);
		}
	}
	return result;
}

std::vector<HEVertex *> HEMesh::getHalfVertsFromVertex(HEVertex *vert) const {
	std::vector<HEVertex *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return {};

	for (auto *pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.push_back(pEdge->last);
		}
	}
	return result;
}

std::vector<HEEdge *> HEMesh::getEdgesFromVertex(HEVertex *vert) const {
	std::vector<HEEdge *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return result;

	for (const auto &pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert || pEdge->last == vert)
				result.push_back(pEdge);
		}
	}
	return result;
}

std::vector<HEEdge *> HEMesh::getHalfEdgesFromVertex(HEVertex *vert) const {
	std::vector<HEEdge *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return result;

	for (const auto &pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.push_back(pEdge);
		}
	}
	return result;
}


HalfEdge::HEVertex *HEMesh::getVertex(size_t idx) const {
	assert(idx < verts.size());
	return verts[idx].get();
}


size_t HEMesh::getVertexFaceCount(HEVertex *pVert) const {
	if (pVert == nullptr)
		return 0;
	auto iter = faceMap.find(pVert);
	if (iter != faceMap.end())
		return iter->second.size();
	return 0;
}

bool HEMesh::hasFace() const {
	return !faceMap.empty();
}

HEVertex *HEMesh::insertVertex(const float3 &position, const float2 &texcoord) {
	verts.push_back(std::make_unique<HEVertex>());
	auto *ptr = verts.back().get();
	ptr->position = position;
	ptr->texcoord = texcoord;
	ptr->index = static_cast<uint32>(verts.size()) - 1u;
	return ptr;
}


HEEdge *HEMesh::insertEdge(HEVertex *v1, HEVertex *v2) {
	edges.emplace_back(std::make_unique<HEEdge>(v1, v2, nullptr));
	return edges.back().get();
}

HEFace *HEMesh::insertFace(const std::array<com::uint32, 3> &indices) {
	HEVertex *pVert0 = verts[indices[0]].get();
	HEVertex *pVert1 = verts[indices[1]].get();
	HEVertex *pVert2 = verts[indices[2]].get();
	HEEdge *pEdge0 = insertEdge(pVert0, pVert1);
	HEEdge *pEdge1 = insertEdge(pVert1, pVert2);
	HEEdge *pEdge2 = insertEdge(pVert2, pVert0);
	faces.emplace_back(std::make_unique<HEFace>());
	HEFace *pFace = faces.back().get();
	pFace->edges = { pEdge0, pEdge1, pEdge2 };
	pEdge0->face = pEdge1->face = pEdge2->face = pFace;
	faceMap[pVert0].push_back(pFace);
	faceMap[pVert1].push_back(pFace);
	faceMap[pVert2].push_back(pFace);
	return pFace;
}

}