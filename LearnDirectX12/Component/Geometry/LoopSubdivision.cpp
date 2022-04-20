#include "LoopSubdivision.h"
#include "Math/MathHelper.h"
#include <unordered_set>
#include <iostream>
#include <map>

namespace loop {


bool operator==(const LoopEdge &lhs, const LoopEdge &rhs) {
	return lhs.v0 == rhs.v0 && lhs.v1 == rhs.v1 ||
		   lhs.v0 == rhs.v1 && lhs.v1 == rhs.v0;
}

std::size_t LoopEdgeHasher::operator()(const LoopEdge &edge) const {
	return std::hash<uint32>()(edge.v0) ^ std::hash<uint32>()(edge.v1);
}

#if 0
com::MeshData LoopSubdivision::subdivision(const com::MeshData &mesh, int numSubdiv, bool genNrmTan) {
	return subdivision(mesh.vertices, mesh.indices, numSubdiv);
}

com::MeshData LoopSubdivision::subdivision(const std::vector<Vertex> &vertices, 
	const std::vector<uint32> &indices, int numSubdiv, bool genNrmTan)
{
	using std::swap;
	MeshData ret;
	for (int i = 0; i < numSubdiv; ++i) {
		MeshData mesh;
		Input input = i == 0 ? Input{ vertices, indices } : Input{ ret.vertices, ret.indices };
		Output output = { mesh.vertices, mesh.indices };
		insertVert(input, output);
		auto vertSource = insertFace(input, output);
		updateBoundary();
		adjustOriginVert(input, output);
		adjustNewVert(input, output, vertSource);
		swap(mesh, ret);
	}

	if (genNrmTan) {
		com::GometryGenerator gen;
		gen.generateTangentAndNormal(ret);
	}
	return ret;
}

const std::set<uint32> &LoopSubdivision::getNeighborsVert(uint32 vert) const {
	assert(vert < neighbors_.size());
	return neighbors_[vert];
}

std::vector<uint32> LoopSubdivision::getNeighborsBoundaryVert(uint32 vert) const {
	assert(vert < neighbors_.size());
	std::vector<uint32> ret;
	for (uint32 neiVert : neighbors_[vert]) {
		if (boundarys_[neiVert]) {
			LoopEdge edge = { vert, neiVert };
			if (auto iter = edgeRefCount_.find(edge); iter != edgeRefCount_.end() && iter->second == 1)
				ret.push_back(neiVert);
		}
	}
	return ret;
}

std::vector<uint32> LoopSubdivision::getSharePoint(uint32 v1, uint32 v2) const {
	assert(v1 < neighbors_.size());
	assert(v2 < neighbors_.size());
	std::vector<uint32> ret;
	std::map<uint32, int> counter;
	for (uint32 idx : neighbors_[v1])
		++counter[idx];
	for (uint32 idx : neighbors_[v2]) {
		auto iter = counter.find(idx);
		if (iter != counter.end()) {
			++iter->second;
			ret.push_back(idx);
		}
	}
	return ret;
}

Vertex LoopSubdivision::middlePoint(const Vertex &lhs, const Vertex &rhs) {
	return {
		float3((Vector3(lhs.position) + Vector3(rhs.position)) * 0.5f),
		(lhs.texcoord + rhs.texcoord) * 0.5f,
	};
}

void LoopSubdivision::adjustOriginVert(Input input, Output output) {
	for (uint32 i = 0; i < input.vertices.size(); ++i) {
		if (boundarys_[i]) {
			auto neiVerts = getNeighborsBoundaryVert(i);
			assert(neiVerts.size() == 2);
			constexpr float _6_div_8 = 6.f / 8.f;
			constexpr float _1_div_8 = 1.f / 8.f;
			float3 pos = input.vertices[i].position * _6_div_8;
			float2 tex = input.vertices[i].texcoord * _6_div_8;
			pos += input.vertices[neiVerts[0]].position * _1_div_8;
			tex += input.vertices[neiVerts[0]].texcoord * _1_div_8;
			pos += input.vertices[neiVerts[1]].position * _1_div_8;
			tex += input.vertices[neiVerts[1]].texcoord * _1_div_8;
			output.vertices[i].position = pos;
			output.vertices[i].texcoord = tex;
		} else {
			auto neiVerts = getNeighborsVert(i);
			float n = static_cast<float>(neiVerts.size());
			constexpr float _5_div_8 = 5.f / 8.f;
			constexpr float _3_div_8 = 3.f / 8.f;
			constexpr float _1_div_4 = 1.f / 4.f;
			float b = std::cos(DX::XM_2PI / 4 * n);
			float beta = (_5_div_8 - (_3_div_8 + b * b)) / n;
			float ratio = (1.f - n * beta);
			float3 pos = input.vertices[i].position * ratio;
			float2 tex = input.vertices[i].texcoord * ratio;
			for (uint32 idx : neiVerts) {
				pos += input.vertices[i].position * beta;
				tex += input.vertices[i].texcoord * beta;
			}
			output.vertices[i].position = pos;
			output.vertices[i].texcoord = tex;
		}
	}
}

void LoopSubdivision::adjustNewVert(Input input, Output output, 
	const std::unordered_map<uint32, LoopEdge>& vertSource)
{
	for (uint32 i = uint32(input.vertices.size()); i < output.vertices.size(); ++i) {
		LoopEdge edge;
		if (auto iter = vertSource.find(i); iter != vertSource.end())
			edge = iter->second;
		else
			continue;

		// 从边界边创建出来的顶点, 不用处理
		if (boundarys_[edge.v0] && boundarys_[edge.v1])
			continue;

		auto sharePoints = getSharePoint(edge.v0, edge.v1);
		if (sharePoints.size() != 2) {
			std::cout << "I: " << i << std::endl;
			continue;
		}

		constexpr float _3_div_8 = 3.f / 8.f;
		constexpr float _1_div_8 = 1.f / 8.f;
		float3 pos = input.vertices[edge.v0].position * _3_div_8; 
		float2 tex = input.vertices[edge.v0].texcoord * _3_div_8; 
		pos += input.vertices[edge.v1].position * _3_div_8;
		tex += input.vertices[edge.v1].texcoord * _3_div_8;
		pos += input.vertices[sharePoints[0]].position * _1_div_8;
		tex += input.vertices[sharePoints[0]].texcoord * _1_div_8;
		pos += input.vertices[sharePoints[1]].position * _1_div_8;
		tex += input.vertices[sharePoints[1]].texcoord * _1_div_8;
		output.vertices[i].position = pos;
		output.vertices[i].texcoord = tex;
	}
}


void LoopSubdivision::insertVert(Input input, Output output) {
	output.vertices.reserve(input.vertices.size() + input.indices.size());
	boundarys_.clear();
	boundarys_.resize(input.vertices.size(), false);
	neighbors_.clear();
	neighbors_.resize(input.vertices.size());
	for (auto &vert : input.vertices)
		output.vertices.push_back(Vertex{ vert.position, vert.texcoord });
}

std::unordered_map<uint32, LoopEdge> LoopSubdivision::insertFace(Input input, Output output) {
	edgeRefCount_.clear();
	output.indices.reserve(input.indices.size() * 4);

	std::unordered_map<LoopEdge, uint32, LoopEdgeHasher> newVert;
	auto createNewVert = [&](LoopEdge edge) -> uint32 {
		if (auto iter = newVert.find(edge); iter != newVert.end())
			return iter->second;
		
		output.vertices.push_back(middlePoint(input.vertices[edge.v0], input.vertices[edge.v1]));
		auto idx = static_cast<uint32>(output.vertices.size() - 1);
		newVert.insert(std::make_pair(edge, idx));
		return idx;
	};

	std::unordered_map<uint32, LoopEdge> vertSource;
	for (size_t i = 0; i < input.indices.size()-2; i += 3) {
		uint32 idx0 = input.indices[i + 0];
		uint32 idx1 = input.indices[i + 1];
		uint32 idx2 = input.indices[i + 2];
		LoopEdge e0 = { idx0, idx1 };
		LoopEdge e1 = { idx1, idx2 };
		LoopEdge e2 = { idx2, idx0 };

		++edgeRefCount_[e0];
		++edgeRefCount_[e1];
		++edgeRefCount_[e2];

		neighbors_[idx0].insert({ idx1, idx2 });
		neighbors_[idx1].insert({ idx2, idx0 });
		neighbors_[idx2].insert({ idx0, idx1 });

		uint32 newIdx0 = createNewVert(e0);
		uint32 newIdx1 = createNewVert(e1);
		uint32 newIdx2 = createNewVert(e2);
		output.indices.insert(output.indices.end(), { idx0, newIdx0, newIdx2 });
		output.indices.insert(output.indices.end(), { newIdx0, idx1, newIdx1 });
		output.indices.insert(output.indices.end(), { newIdx1, idx2, newIdx2 });
		output.indices.insert(output.indices.end(), { newIdx0, newIdx1, newIdx2 });

		vertSource[newIdx0] = e0;
		vertSource[newIdx1] = e1;
		vertSource[newIdx2] = e2;
	}
	return vertSource;
}

void LoopSubdivision::updateBoundary() {
	for (auto &&[edge, count] : edgeRefCount_) {
		if (count == 1) {
			boundarys_[edge.v0] = true;
			boundarys_[edge.v1] = true;
		}
	}
}

#endif

}