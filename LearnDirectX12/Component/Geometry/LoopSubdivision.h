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
	std::vector<bool> boundarys_;
	std::vector<std::set<uint32>> neighbors_;
	std::unordered_map<LoopEdge, int, LoopEdgeHasher> edgeRefCount_;
public:
	com::MeshData subdivision(const std::vector<Vertex>& inputVert, const std::vector<uint32>& inputIdx, 
		int numSubdiv = 1, bool genNrmTan = false);
	com::MeshData subdivision(const com::MeshData &mesh, 
		int numSubdiv = 1, bool genNrmTan = false);
private:
	struct Input {
		const std::vector<Vertex> &vertices;
		const std::vector<uint32> &indices;
	};
	struct Output {
		std::vector<Vertex> &vertices;
		std::vector<uint32> &indices;
	};

	const std::set<uint32> &getNeighborsVert(uint32 vert) const;
	std::vector<uint32> getNeighborsBoundaryVert(uint32 vert) const;
	std::vector<uint32> getSharePoint(uint32 v1, uint32 v2) const;
	static Vertex middlePoint(const Vertex &lhs, const Vertex &rhs);

	void adjustOriginVert(Input input, Output output);
	void adjustNewVert(Input input, Output output, const std::unordered_map<uint32, LoopEdge> &vertSource);
	void insertVert(Input input, Output output);
	void updateBoundary();
	std::unordered_map<uint32, LoopEdge> insertFace(Input input, Output output);
};

}
