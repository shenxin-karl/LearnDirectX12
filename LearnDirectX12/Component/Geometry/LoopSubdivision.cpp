#include "LoopSubdivision.h"
#include "Math/MathHelper.h"

namespace com {

com::MeshData LoopSubdivision::subdivide2MeshData(const MeshData &mesh) {
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	indices.reserve(mesh.indices.size() * 4);
	vertices.resize(mesh.vertices.size() * 2);

	for (const auto &vert : mesh.vertices) {
		hemesh_.insertVertex(vert.position, vert.texcoord);
		vertices.emplace_back(vert);
	}

	std::unordered_map<uint32, std::pair<uint32, uint32>> deriveIndex;
	for (size_t i = 0; i < mesh.indices.size()-2; i += 3) {
		uint32 idx0 = mesh.indices[i+0];
		uint32 idx1 = mesh.indices[i+1];
		uint32 idx2 = mesh.indices[i+2];
		hemesh_.insertFace({ idx0, idx1, idx2 });
		uint32 baseIdx = static_cast<uint32>(vertices.size());
		deriveIndex[baseIdx+0] = { idx0, idx1 };
		deriveIndex[baseIdx+1] = { idx1, idx2 };
		deriveIndex[baseIdx+2] = { idx2, idx0 };
		indices.insert(indices.end(), { idx0, baseIdx+0, baseIdx+2 });
		indices.insert(indices.end(), { baseIdx+0, idx1, baseIdx+1 });
		indices.insert(indices.end(), { baseIdx+1, idx2, baseIdx+2 });
		indices.insert(indices.end(), { baseIdx+0, baseIdx+1, baseIdx+2 });
	}
	 
	adjustOriginVertex(vertices, mesh.vertices.size());
	adjustNewVertex(vertices, mesh.vertices.size());
	return { std::move(vertices), std::move(indices) };
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
	float Beta = (5.f / 8.f - std::pow(3.f / 8.f + std::cos(DX::XM_2PI / 4.f), 2.f));
	for (uint32 i = 0; i < num; ++i) {
		HE::HEVertex *pHeVert = hemesh_.getVertex(i);
		std::vector<HE::HEVertex *> halfEdgeVert = hemesh_.getHalfVertsFromVertex(pHeVert);
		if (hemesh_.getVertexFaceCount(pHeVert) <= 1) {		// boundary
			assert(halfEdgeVert.size() != 2);
			float3 position = pHeVert->position * (6.f / 8.f);
			float2 texcoord = pHeVert->texcoord * (6.f / 8.f);
			for (const auto *pVert : halfEdgeVert) {
				position += pVert->position * (1.f / 8.f);
				texcoord += pVert->texcoord * (1.f / 8.f);
			}
			vertices[i].position = position;
			vertices[i].texcoord = texcoord;
		} else {
			float beta = Beta / halfEdgeVert.size();
			float3 otherPos = float3(0);
			float2 otherTex = float2(0);
			for (const auto *pVert : halfEdgeVert) {
				otherPos += pVert->position;
				otherTex += pVert->texcoord;
			}
			vertices[i].position = pHeVert->position * (1.f - beta) + beta * otherPos;
			vertices[i].texcoord = pHeVert->texcoord * (1.f - beta) + beta * otherTex;
		}
	}
}

void LoopSubdivision::adjustNewVertex(std::vector<Vertex> &vertices, size_t first) const {
	 
}

}