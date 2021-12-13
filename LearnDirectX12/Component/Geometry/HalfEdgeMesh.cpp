#include "HalfEdgeMesh.h"
#include <unordered_set>
#include <iostream>

namespace HalfEdge {

std::size_t HEEdgeKeyHasher::operator()(const HEEdgeKey &key) const noexcept {
	using PointerType = decltype(key.first);
	return std::hash<PointerType>()(key.first) ^ (std::hash<PointerType>()(key.second));
}

void HEMesh::updateVertBoundaryInfo() const {
	if (!dirty)
		return;

	for (auto &pEdge : halfEdges) {
		HEEdgeKey key(pEdge->start, pEdge->last);
		auto iter = edgeRefInfo.find(key);
		assert(iter != edgeRefInfo.end());
		pEdge->isBoundary = iter->second.refCount == 1;
	}
	dirty = false;
}

HEMesh::HEMesh(const com::MeshData &mesh) {
	for (auto &vert : mesh.vertices)
		insertVertex(vert.position, vert.texcoord);

	for (com::uint32 i = 0; i < mesh.indices.size()-2; i += 3) {
		com::uint32 idx0 = mesh.indices[i + static_cast<std::size_t>(0)];
		com::uint32 idx1 = mesh.indices[i + static_cast<std::size_t>(1)];
		com::uint32	idx2 = mesh.indices[i + static_cast<std::size_t>(2)];
		insertFace({ idx0, idx1, idx2 });
	}
}

HEMesh::HEMesh(const std::vector<HEVertex> &vertices, std::vector<com::uint32> &indices) {
	for (auto &vert : vertices)
		insertVertex(vert.position, vert.texcoord);

	for (com::uint32 i = 0; i < indices.size()-2; i += 3) {
		com::uint32 idx0 = indices[i + static_cast<std::size_t>(0)];
		com::uint32 idx1 = indices[i + static_cast<std::size_t>(1)];
		com::uint32	idx2 = indices[i + static_cast<std::size_t>(2)];
		insertFace({ idx0, idx1, idx2 });
	}
}

void HEMesh::foreachFace(const std::function<void(const HEMesh *, const HEFace *)> &callback) const {
	for (auto &face : faces)
		callback(this, face.get());
}


void HEMesh::foreachFace(const HEVertex *pVert, const std::function<void(HEFace *)> &callback) const {
	for (auto *pFace : getFace(pVert))
		callback(pFace);
}

void HEMesh::foreachNeighborsVerts(const HEVertex *pVert, const std::function<void(HEVertex *)> &callback) const {
	for (auto *pFace : getFace(pVert)) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == pVert || pEdge->last == pVert) {
				if (pEdge->start == pVert)
					callback(pEdge->last);
				else
					callback(pEdge->start);
			}
		}
	}
}

void HEMesh::foreachNeighborsHalfVerts(const HEVertex *pVert, const std::function<void(HEVertex *)> &callback) const {
	for (auto *pFace : getFace(pVert)) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == pVert)
				callback(pEdge->last);
		}
	}
}

void HEMesh::foreachNeighborsEdges(const HEVertex *pVert, const std::function<void(HEEdge *)> &callback) const {
	for (auto *pFace : getFace(pVert)) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == pVert || pEdge->last == pVert)
				callback(pEdge);
		}
	}
}

void HEMesh::foreachNeighborsHalfEdges(const HEVertex *pVert, const std::function<void(HEEdge *)> &callback) const {
	for (auto *pFace : getFace(pVert)) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == pVert)
				callback(pEdge);
		}
	}
}

std::vector<HEFace *> HEMesh::getFace(const HEVertex *vert) const {
	if (auto iter = vert2Face.find(vert); iter != vert2Face.end())
		return iter->second;
	return {};
}

std::vector<HEVertex *> HEMesh::getNeighborsVerts(const HEVertex *vert) const {
	std::vector<HEVertex *> result;
	foreachNeighborsVerts(vert, [&](HEVertex *pOtherVert) {
		result.push_back(pOtherVert);
	});
	return result;
}

std::vector<HEVertex *> HEMesh::getNeighborsHalfVerts(const HEVertex *vert) const {
	std::vector<HEVertex *> result;
	foreachNeighborsHalfVerts(vert, [&](HEVertex *pOtherVert) {
		result.push_back(pOtherVert);
	});
	return result;
}

std::vector<HEEdge *> HEMesh::getNeighborsEdges(const HEVertex *vert) const {
	std::vector<HEEdge *> result;
	foreachNeighborsEdges(vert, [&](HEEdge *pOhterVert) {
		result.push_back(pOhterVert);
	});
	return result;
}

std::vector<HEEdge *> HEMesh::getNeighborsHalfEdges(const HEVertex *vert) const {
	std::vector<HEEdge *> result;
	foreachNeighborsHalfEdges(vert, [&](HEEdge *pOtherVert) {
		result.push_back(pOtherVert);
	});
	return result;
}

HalfEdge::HEVertex *HEMesh::getVertex(size_t idx) const {
	assert(idx < verts.size());
	return verts[idx].get();
}

bool HEMesh::isBoundaryVert(const HEVertex *pVert) const {
	updateVertBoundaryInfo();
	for (auto *pEdge : getNeighborsEdges(pVert)) {
		if (pEdge->isBoundary)
			return true;
	}
	return false;
}

bool HEMesh::isBoundaryEdge(const HEVertex *pVert1, const HEVertex *pVert2) const {
	updateVertBoundaryInfo();
	HEEdgeKey key(pVert1, pVert2);
	if (auto iter = edgeRefInfo.find(key); iter != edgeRefInfo.end())
		return iter->second.halfEdge->isBoundary;
	return false;
}

bool HEMesh::isBoundaryEdge(const HEEdge *pEdge) const {
	updateVertBoundaryInfo();
	return pEdge->isBoundary;
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
	auto key = HEEdgeKey(v1, v2);
	halfEdges.emplace_back(std::make_unique<HEEdge>(v1, v2, nullptr, 1));
	auto *pEdge = halfEdges.back().get();
	edgeRefInfo[key].refCount++;
	edgeRefInfo[key].halfEdge = pEdge;
	dirty = true;
	return pEdge;
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
	vert2Face[pVert0].push_back(pFace);
	vert2Face[pVert1].push_back(pFace);
	vert2Face[pVert2].push_back(pFace);
	return pFace;
}

void swap(HEMesh &lhs, HEMesh &rhs) {
	using std::swap;
	swap(lhs.verts, rhs.verts);
	swap(lhs.faces, rhs.faces);
	swap(lhs.halfEdges, rhs.halfEdges);
	swap(lhs.vert2Face, rhs.vert2Face);
}

}
