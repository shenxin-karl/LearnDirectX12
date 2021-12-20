#include "LoopSubdivision.h"
#include "Math/MathHelper.h"
#include <unordered_set>
#include <iostream>

namespace loop {

bool operator==(const LoopEdge &lhs, const LoopEdge &rhs) {
	return lhs.v0 == rhs.v0 && lhs.v1 == rhs.v1 ||
		   lhs.v0 == rhs.v1 && lhs.v1 == rhs.v0;
}

std::size_t LoopEdgeHasher::operator()(const LoopEdge &edge) const {
	return std::hash<uint32>()(edge.v0) ^ std::hash<uint32>()(edge.v1);
}

com::MeshData LoopSubdivision::subdivision(const com::MeshData &mesh, int numSubdiv) {
	verts_.reserve(mesh.vertices.size());
	neighbors_.resize(mesh.vertices.size());
	boundarys_.resize(mesh.vertices.size(), false);
	for (const auto &vert : mesh.vertices)
		verts_.push_back(LoopVert{ vert.position, vert.texcoord });

	std::vector<Vertex> vertices = mesh.vertices;
	vertices.reserve(mesh.indices.size());
	std::vector<uint32> indices = insertFace(vertices, mesh.indices);
	for (auto &&[edge, count] : edgeRefCount_) {
		if (count == 1) {
			boundarys_[edge.v0] = true;
			boundarys_[edge.v1] = true;
		}
	}

	uint32 vertCount = mesh.vertices.size();
	adjustOriginVert(vertices, vertCount);
	adjustNewVert(vertices, vertCount);

}

const std::set<uint32> &LoopSubdivision::getNeighborsVert(uint32 vert) {
	assert(vert >= neighbors_.size());
	return neighbors_[vert];
}

std::vector<uint32> LoopSubdivision::getNeighborsBoundaryVert(uint32 vert) {
	assert(vert >= neighbors_.size());
	std::vector<uint32> ret;
	for (uint32 neiVert : neighbors_[vert]) {
		if (boundarys_[neiVert])
			ret.push_back(neiVert);
	}
	return ret;
}

Vertex LoopSubdivision::middlePoint(const Vertex &lhs, const Vertex &rhs) {
	return {
		MathHelper::lerp(lhs.position, rhs.position, 0.5f),
		MathHelper::lerp(lhs.texcoord, rhs.texcoord, 0.5f),
	};
}

void LoopSubdivision::adjustOriginVert(std::vector<Vertex> &vertices, uint32 count) {
	for (uint32 i = 0; i < count; ++i) {
		if (boundarys_[i]) {
			auto neiVerts = getNeighborsBoundaryVert(i);
			assert(neiVerts.size() == 2);
			constexpr float _1_div_6 = 1.f / 6.f;
			constexpr float _1_div_8 = 1.f / 8.f;
			float3 pos = verts_[i].position * _1_div_6;
			float2 tex = verts_[i].texcoord * _1_div_6;
			for (uint32 idx : neiVerts) {
				pos += verts_[idx].position * _1_div_8;
				tex += verts_[idx].texcoord * _1_div_8;
			}
			vertices[i].position = pos;
			vertices[i].texcoord = tex;
		} else {
			auto neiVerts = getNeighborsVert(i);
			float n = static_cast<float>(neiVerts.size());
			constexpr float _5_div_8 = 5.f / 8.f;
			constexpr float _3_div_8 = 3.f / 8.f;
			constexpr float _1_div_4 = 1.f / 4.f;
			float b = std::cos(DX::XM_2PI / 4 * n);
			float beta = (_5_div_8 - (_3_div_8 + b * b)) / n;
			float ratio = (1.f - n * beta);
			float3 pos = verts_[i].position * ratio;
			float2 tex = verts_[i].texcoord * ratio;
			for (uint32 idx : neiVerts) {
				pos += verts_[i].position * beta;
				tex += verts_[i].texcoord * beta;
			}
		}
	}
}

std::vector<uint32> LoopSubdivision::insertFace(std::vector<Vertex> &vertices, const std::vector<uint32> &indices) {
	std::vector<uint32> ret;
	ret.reserve(indices.size() * 3);
	for (size_t i = 0; i < indices.size(); i += 3) {
		uint32 idx0 = indices[i + 0];
		uint32 idx1 = indices[i + 1];
		uint32 idx2 = indices[i + 2];
		LoopEdge e0 = { idx0, idx1 };
		LoopEdge e1 = { idx1, idx2 };
		LoopEdge e2 = { idx2, idx0 };
		++edgeRefCount_[e0];
		++edgeRefCount_[e1];
		++edgeRefCount_[e2];
		neighbors_[idx0].insert({ idx1, idx2 });
		neighbors_[idx1].insert({ idx2, idx0 });
		neighbors_[idx2].insert({ idx0, idx1 });
		uint32 baseIdx = static_cast<uint32>(vertices.size());
		vertices.push_back(middlePoint(vertices[idx0], vertices[idx1]));
		vertices.push_back(middlePoint(vertices[idx1], vertices[idx2]));
		vertices.push_back(middlePoint(vertices[idx2], vertices[idx0]));
		ret.insert(ret.end(), { idx0, baseIdx + 0, baseIdx + 2 });
		ret.insert(ret.end(), { baseIdx + 0, idx1, baseIdx + 1 });
		ret.insert(ret.end(), { baseIdx + 1, idx2, baseIdx + 2 });
		ret.insert(ret.end(), { baseIdx + 0, baseIdx + 1, baseIdx + 1 });
	}
	return ret;
}

}