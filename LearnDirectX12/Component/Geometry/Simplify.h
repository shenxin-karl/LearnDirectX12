#pragma once
#include <vector>
#include "Geometry/GeometryGenerator.h"
#include "Math/VectorHelper.h"
#include <set>
#include <unordered_set>
#include <queue>


namespace sim {

using namespace vec;

struct SimVertex {
	float3	position;
	float2	texcoord;
};

struct SimEdge {
	int	start;
	int last;
public:
	friend bool operator==(const SimEdge &lhs, const SimEdge &rhs);
};

struct simEdgeHasher {
	std::size_t operator()(const SimEdge &edge) const noexcept;
};

struct CostRecord {
	float	cost;
	SimEdge edge;
public:
	friend bool operator<(const CostRecord &lhs, const CostRecord &rhs);
};

struct SimFace {
	int v1;		
	int v2;
};

struct SimVertAdjustResult {
	float  cost;
	float3 point;
};

class Simplify {
	std::vector<SimVertex>							vertices_;
	std::vector<unsigned char>						removed_;
	std::vector<std::vector<SimFace>>				faces_;
	std::unordered_set<SimEdge, simEdgeHasher>		edges_;
	std::set<CostRecord>							heap_;
	float											thresholdSqr_ = 0.f;
public:
	Simplify(float threshold, const std::vector<com::Vertex> &vertices, const std::vector<com::uint32> &indices);
	void simplify(float target);
	com::MeshData unloadData() const;
private:
	void buildHeap();
	SimVertAdjustResult calcAdjustEdgeResult(const SimEdge &edge) const;
	float calcEdgeLengthSqr(const SimEdge &edge) const;
	void pushHeap(const SimEdge &edge);
	CostRecord popHeap();
};

}
