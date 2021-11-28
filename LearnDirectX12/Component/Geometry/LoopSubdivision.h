#pragma once
#include "GeometryGenerator.h"
#include "HalfEdgeMesh.h"

namespace com {

namespace HE = HalfEdge;

class LoopSubdivision {
	HE::HEMesh	hemesh_;
public:
	LoopSubdivision() = default;
	MeshData subdivide2MeshData(const MeshData &mesh);
	HE::HEMesh subdivide2HEMesh();
	static Vertex middle(const Vertex &lhs, const Vertex &rhs);
private:
	void adjustOriginVertex(std::vector<Vertex> &vertices, size_t num) const;
	void adjustNewVertex(std::vector<Vertex> &vertices, size_t first, 
		const std::unordered_map<uint32, std::pair<uint32, uint32>> &deriedIndex) const;
	void generateNewVertex(std::vector<Vertex> &vertices, std::vector<uint32> &indices, 
		uint32 idx0, uint32 idx1, uint32 idx2);
};

}