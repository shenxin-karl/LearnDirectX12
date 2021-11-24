#pragma once
#include "GeometryGenerator.h"
#include "HalfEdgeMesh.h"

namespace com {

namespace HE = HalfEdge;

class LoopSubdivision {
	HE::HEMesh	hemesh_;
public:
	MeshData subdivide2MeshData(const MeshData &mesh);
	HE::HEMesh subdivide2HEMesh();
	static Vertex middle(const Vertex &lhs, const Vertex &rhs);
private:
	void adjustOriginVertex(std::vector<Vertex> &vertices, size_t num) const;
	void adjustNewVertex(std::vector<Vertex> &vertices, size_t first) const;
};

}