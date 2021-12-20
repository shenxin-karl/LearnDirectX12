#pragma once
#include "GeometryGenerator.h"
#include <unordered_map>
#include <set>
#include <functional>

namespace loop {

using namespace vec;
using namespace com;

struct LoopVert {
	float3	position;
	float2	texcoord;
};

struct LoopEdge {
	uint32 v0;
	uint32 v1;
public:
	friend bool operator==(const LoopEdge &lsh, const LoopEdge &rhs);
};

struct LoopEdgeHasher {
	std::size_t operator()(const LoopEdge &edge) const;
};

struct LoopFace {
	uint32 v1;
	uint32 v2;
};

class LoopSubdivision {
	std::vector<LoopVert> verts_;
	std::vector<bool>	  boundarys_;
	std::vector<std::set<uint32>> neighbors_;
	std::unordered_map<LoopEdge, int> edgeRefCount_;
public:
	com::MeshData subdivision(const com::MeshData &mesh, int numSubdiv = 1);
private:
	const std::set<uint32> &getNeighborsVert(uint32 vert);
	std::vector<uint32> getNeighborsBoundaryVert(uint32 vert);
	static Vertex middlePoint(const Vertex &lhs, const Vertex &rhs);
	void adjustOriginVert(std::vector<Vertex> &vertices, uint32 count);
	void adjustNewVert(std::vector<Vertex> &vertices, uint32 start);
	std::vector<uint32> insertFace(std::vector<Vertex> &vertices, const std::vector<uint32> &indices);
};

}
