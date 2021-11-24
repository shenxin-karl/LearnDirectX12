#pragma once
#include "GeometryGenerator.h"
#include "HalfEdgeMesh.h"

namespace com {

namespace HE = HalfEdge;

class LoopSubdivision {
	HE::HEMesh	hemesh_;
public:
	MeshData subdivide2MeshData(const MeshData &mesh) const;
	HE::HEMesh subdivide2HEMesh() const;
};

}