#include "HalfEdgeMesh.h"
#include <unordered_set>
#include <iostream>

namespace HalfEdge {

std::size_t HEEdgeKeyHasher::operator()(const HEEdgeKey &key) const noexcept {
	using PointerType = decltype(key.first);
	return std::hash<PointerType>()(key.first) ^ (std::hash<PointerType>()(key.second) << 1);
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

HEMesh::HEMesh(const HEMesh &other) {
	for (auto &vert : other.verts)
		insertVertex(vert->position, vert->texcoord);

	other.foreachFace([](const HEMesh *pMesh, const HEFace *pFace) {
		std::vector<int> indices;
		indices.reserve(3);
		for (auto *pEdge : pFace->edges)
			indices.push_back(pEdge->start->index);
	});
}


void HEMesh::foreachFace(const std::function<void(const HEMesh *, const HEFace *)> &callback) const {
	for (auto &face : faces)
		callback(this, face.get());
}

std::vector<HEFace *> HEMesh::getFaceFromVertex(const HEVertex *vert) const {
	std::unordered_set<HEFace *> result;
	if (auto iter = faceMap.find(vert); iter != faceMap.end())
		return iter->second;
	return {};
}

std::unordered_set<HEVertex *> HEMesh::getVertsFromVertex(const HEVertex *vert) const {
	std::unordered_set<HEVertex *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return {};

	for (auto *pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.insert(pEdge->last);
			else if (pEdge->last == vert)
				result.insert(pEdge->start);
		}
	}
	return result;
}

std::unordered_set<HEVertex *> HEMesh::getHalfVertsFromVertex(const HEVertex *vert) const {
	std::unordered_set<HEVertex *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return {};

	for (auto *pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.insert(pEdge->last);
		}
	}
	return result;
}

std::unordered_set<HEEdge *> HEMesh::getEdgesFromVertex(const HEVertex *vert) const {
	std::unordered_set<HEEdge *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return result;

	for (const auto &pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert || pEdge->last == vert)
				result.insert(pEdge);
		}
	}
	return result;
}

std::unordered_set<HEEdge *> HEMesh::getHalfEdgesFromVertex(const HEVertex *vert) const {
	std::unordered_set<HEEdge *> result;
	auto iter = faceMap.find(vert);
	if (iter == faceMap.end())
		return result;

	for (const auto &pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (pEdge->start == vert)
				result.insert(pEdge);
		}
	}
	return result;
}


std::unordered_set<HEVertex *> HEMesh::getUnionVert(const HEVertex *pVert1, const HEVertex *pVert2) const {
	auto iter1 = faceMap.find(pVert1);
	auto iter2 = faceMap.find(pVert2);
	if (iter1 == faceMap.end() || iter2 == faceMap.end())
		return {};
	
	std::unordered_map<HEFace *, int> faceHashmap;
	std::unordered_set<HEVertex *> result;
	for (auto *pFace : iter1->second)
		faceHashmap[pFace]++;
	for (auto *pFace : iter2->second)
		faceHashmap[pFace]++;

	for (auto &&[pFace, num] : faceHashmap) {
		if (num <= 1)
			continue;
		for (auto *pEdge : pFace->edges) {
			HEVertex *pVert = nullptr;
			if (pEdge->start == pVert1 || pEdge->start == pVert2)
				pVert = pEdge->last;
			else if (pEdge->last == pVert1 || pEdge->last == pVert2)
				pVert = pEdge->start;
			if (pVert != pVert1 && pVert != pVert2)
				result.insert(pVert);
		}
	}
	return result;
}


std::unordered_set<HEVertex *> HEMesh::getNeighborsVertFromVert(const HEVertex *pVert) const {
	if (!isBoundaryVert(pVert))
		return {};

	auto iter = faceMap.find(pVert);
	if (iter == faceMap.end())
		return {};


	std::unordered_set<HEVertex *> result;
	for (auto *pFace : iter->second) {
		for (auto *pEdge : pFace->edges) {
			if (isBoundaryEdge(pEdge)) {
				if (pEdge->start == pVert && !isPairEdge(pEdge))
					result.insert(pEdge->last);
				else if (pEdge->last == pVert && !isPairEdge(pEdge))
					result.insert(pEdge->start);
			}
		}
	}
	return result;
}

HalfEdge::HEVertex *HEMesh::getVertex(size_t idx) const {
	assert(idx < verts.size());
	return verts[idx].get();
}


size_t HEMesh::getVertexFaceCount(const HEVertex *pVert) const {
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


bool HEMesh::isBoundaryVert(const HEVertex *pVert) const {
	return pVert->isBoundary;
}


bool HEMesh::isBoundaryEdge(const HEVertex *pVert1, const HEVertex *pVert2) const {
	return pVert1->isBoundary && pVert2->isBoundary;
}


bool HEMesh::isBoundaryEdge(const HEEdge *pEdge) const {
	return isBoundaryEdge(pEdge->start, pEdge->last);
}


bool HEMesh::isPairEdge(const HEVertex *pVert1, const HEVertex *pVert2) const {
	int flag = 0;
	if (auto iter = edges.find(HEEdgeKey(pVert1, pVert2)); iter != edges.end())
		++flag;
	if (auto iter = edges.find(HEEdgeKey(pVert2, pVert1)); iter != edges.end())
		++flag;
	return flag == 2;
}


bool HEMesh::isPairEdge(const HEEdge *pEdge) const {
	return isPairEdge(pEdge->start, pEdge->last);
}

void HEMesh::updateVertBoundaryInfo() {
	for (auto &&[key, pEdge] : edges) {
		if (pEdge->refNum == 1) {
			auto *pStart = pEdge->start;
			auto *pLast = pEdge->last;
			auto iter = edges.find(HEEdgeKey(pLast, pStart));
			bool isUniqueEdge = iter == edges.end();
			std::cout << "pStart->index: " << pStart->index << " pLast->index: " << pLast->index << std::endl;
			if (isUniqueEdge) {
				pStart->isBoundary = true;
				pLast->isBoundary = true;
			}
		}
	}
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
	if (auto iter = edges.find(key); iter != edges.end()) {
		iter->second->refNum++;
		return iter->second.get();
	}
	auto pair = edges.emplace(std::make_pair(key, std::make_unique<HEEdge>(v1, v2, nullptr, 1)));
	return pair.first->second.get();
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

void swap(HEMesh &lhs, HEMesh &rhs) {
	using std::swap;
	swap(lhs.verts, rhs.verts);
	swap(lhs.faces, rhs.faces);
	swap(lhs.edges, rhs.edges);
	swap(lhs.faceMap, rhs.faceMap);
}

}


