#pragma once
#include "GeometryGenerator.h"
#include "HalfEdgeMesh.h"

namespace com {

namespace HE = HalfEdge;

class LoopSubdivision {
	HE::HEMesh	mesh;
public:
	LoopSubdivision(const MeshData &mesh);
	LoopSubdivision(const HE::HEMesh &mesh);
	MeshData subdivide2MeshData() const;
	HEMesh subdivide2HEMesh() const;
};

}