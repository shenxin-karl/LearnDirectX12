#include "LoopSubdivision.h"
#include "Math/MathHelper.h"
#include <iostream>

namespace com {

com::MeshData LoopSubdivision::subdivide2MeshData(const MeshData &mesh) {
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	indices.reserve(mesh.indices.size() * 4);
	vertices.reserve(mesh.vertices.size() * 2);

	for (const auto &vert : mesh.vertices) {
		hemesh_.insertVertex(vert.position, vert.texcoord);
		vertices.emplace_back(vert);
	}

	std::unordered_map<uint32, std::pair<uint32, uint32>> deriveIndex;
	for (size_t i = 0; i < mesh.indices.size()-2; i += 3) {
		uint32 idx0 = mesh.indices[i+0];
		uint32 idx1 = mesh.indices[i+1];
		uint32 idx2 = mesh.indices[i+2];
		uint32 baseIdx = static_cast<uint32>(vertices.size());
		deriveIndex[baseIdx+0] = { idx0, idx1 };
		deriveIndex[baseIdx+1] = { idx1, idx2 };
		deriveIndex[baseIdx+2] = { idx2, idx0 };
		generateNewVertex(vertices, indices, idx0, idx1, idx2);
	}

	adjustOriginVertex(vertices, mesh.vertices.size());
	adjustNewVertex(vertices, mesh.vertices.size(), deriveIndex);
	MeshData result = { std::move(vertices), std::move(indices) };
	GometryGenerator gen;
	gen.generateTangentAndNormal(result);
	return result;
}


com::Vertex LoopSubdivision::middle(const Vertex &lhs, const Vertex &rhs) {
	return {
		MathHelper::lerp(lhs.position, rhs.position, 0.5f),
		MathHelper::lerp(lhs.texcoord, rhs.texcoord, 0.5f),
		float3(0),
		float3(0),
	};
}


void LoopSubdivision::adjustOriginVertex(std::vector<Vertex> &vertices, size_t num) const {
	//for (uint32 i = 0; i < num; ++i) {
	//	HE::HEVertex *pHeVert = hemesh_.getVertex(i);
	//	if (hemesh_.isBoundaryVert(pHeVert)) {
	//		auto neighborsBoundaryVerts = hemesh_.getNeighborsVertFromVert(pHeVert);
	//		if (neighborsBoundaryVerts.size() != 2) {
	//			std::cerr << "adjustOriginVertex error: " << i << std::endl;
	//			continue;
	//		}
	//		constexpr float _6_div_8 = 6.f / 8.f;
	//		constexpr float _1_div_8 = 1.f / 8.f;
	//		float3 pos = pHeVert->position * _6_div_8;
	//		float2 tex = pHeVert->texcoord * _6_div_8;
	//		for (auto *pOtherVert : neighborsBoundaryVerts) {
	//			pos += pOtherVert->position * _1_div_8;
	//			tex += pOtherVert->texcoord * _1_div_8;
	//		}
	//		vertices[i].position = pos;
	//		vertices[i].texcoord = tex;
	//	} else {
	//		constexpr float _5_div_8 = 5.f / 8.f;
	//		constexpr float _3_div_8 = 3.f / 8.f;
	//		constexpr float _1_div_4 = 1.f / 4.f;
	//		auto verts = hemesh_.getVertsFromVertex(pHeVert);
	//		float n = static_cast<float>(verts.size());
	//		float _1_div_n = 1.f / n;
	//		float alpha = _3_div_8 + _1_div_4 * std::cos(DX::XM_2PI / n);
	//		float beta = _1_div_n * (_5_div_8 - alpha * alpha);
	//		float3 pos = float3(0);
	//		float2 tex = float2(0);
	//		for (auto *pOtherVert : verts) {
	//			pos += pOtherVert->position;
	//			tex += pOtherVert->texcoord;
	//		}
	//		vertices[i].position = pHeVert->position * (1.f - n * beta) + beta * pos;
	//		vertices[i].texcoord = pHeVert->texcoord * (1.f - n * beta) + beta * tex;
	//	}
	//}
}

void LoopSubdivision::adjustNewVertex(std::vector<Vertex> &vertices, size_t first,
	const std::unordered_map<uint32, std::pair<uint32, uint32>> &deriedIndex) const 
{
	//for (uint32 i = static_cast<uint32>(first); i < vertices.size(); ++i) {
	//	auto iter = deriedIndex.find(i);
	//	assert(iter != deriedIndex.end());
	//	const auto &parentIndex = iter->second;
	//	uint32 vertIdx1 = parentIndex.first;
	//	uint32 vertIdx2 = parentIndex.second;
	//	HE::HEVertex *pVert1 = hemesh_.getVertex(vertIdx1);
	//	HE::HEVertex *pVert2 = hemesh_.getVertex(vertIdx2);
	//	if (hemesh_.isBoundaryEdge(pVert1, pVert2))
	//		continue;

	//	auto unionVert = hemesh_.getUnionVert(pVert1, pVert2);
	//	assert(unionVert.size() == 2);
	//	constexpr float _3_div_8 = 3.f / 8.f;
	//	constexpr float _1_div_8 = 1.f / 8.f;
	//	float3 pos = (pVert1->position + pVert2->position) * _3_div_8;
	//	float2 tex = (pVert1->texcoord + pVert2->texcoord) * _3_div_8;
	//	for (auto *pOtherVert : unionVert) {
	//		pos += pOtherVert->position * _1_div_8;
	//		tex += pOtherVert->texcoord * _1_div_8;
	//	}
	//	vertices[i].position = pos;
	//	vertices[i].texcoord = tex;
	//}
}

void LoopSubdivision::generateNewVertex(std::vector<Vertex> &vertices, std::vector<uint32> &indices, 
	uint32 idx0, uint32 idx1, uint32 idx2) 
{
	uint32 baseIdx = static_cast<uint32>(vertices.size());
	const Vertex &v0 = middle(vertices[idx0], vertices[idx1]);
	const Vertex &v1 = middle(vertices[idx1], vertices[idx2]);
	const Vertex &v2 = middle(vertices[idx2], vertices[idx0]);
	hemesh_.insertFace({ idx0, idx1, idx2 });
	//hemesh_.insertVertex(v0.position, v0.texcoord);
	//hemesh_.insertVertex(v1.position, v1.texcoord);
	//hemesh_.insertVertex(v2.position, v2.texcoord);
	vertices.emplace_back(v0);
	vertices.emplace_back(v1);
	vertices.emplace_back(v2);
	indices.insert(indices.end(), { idx0, baseIdx+0, baseIdx+2 });
	indices.insert(indices.end(), { baseIdx+0, idx1, baseIdx+1 });
	indices.insert(indices.end(), { baseIdx+1, idx2, baseIdx+2 });
	indices.insert(indices.end(), { baseIdx+0, baseIdx+1, baseIdx+2 });
	//hemesh_.insertFace({ idx0, baseIdx+0, baseIdx+2 });
	//hemesh_.insertFace({ baseIdx+0, idx1, baseIdx+1 });
	//hemesh_.insertFace({ baseIdx+1, idx2, baseIdx+2 });
	//hemesh_.insertFace({ baseIdx+0, baseIdx+1, baseIdx+2 });
}

}