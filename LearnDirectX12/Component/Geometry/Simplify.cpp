#include "Simplify.h"
#include "Math/Vector.hpp"

namespace sim {

bool operator==(const SimEdge &lhs, const SimEdge &rhs) {
	return lhs.start == rhs.start && lhs.last == rhs.last ||
		lhs.last == rhs.start && lhs.start == rhs.last;
}

std::size_t simEdgeHasher::operator()(const SimEdge &edge) const noexcept {
	return std::hash<int>()(edge.start) ^ std::hash<int>()(edge.last);
}

bool operator<(const CostRecord &lhs, const CostRecord &rhs) {
	return lhs.cost < rhs.cost;
}


sim::Simplify::Simplify(float threshold, const std::vector<com::Vertex> &vertices, 
	const std::vector<com::uint32> &indices) : thresholdSqr_(threshold * threshold)
{
	std::size_t vertSize = vertices.size();
	vertices_.resize(vertSize);
	removed_.resize(vertSize, false);
	faces_.resize(vertSize);

	for (size_t i = 0; i < vertSize; ++i) {
		vertices_[i].position = vertices[i].position;
		vertices_[i].texcoord = vertices_[i].texcoord;
	}

	for (size_t i = 0; i < indices.size()-2; i += 3) {
		com::uint32 idx0 = indices[i+0];
		com::uint32 idx1 = indices[i+1];
		com::uint32 idx2 = indices[i+2];
		faces_[idx0].push_back(SimFace(idx1, idx2));
		faces_[idx1].push_back(SimFace(idx2, idx0));
		faces_[idx2].push_back(SimFace(idx0, idx1));
		edges_.insert(SimEdge(idx1, idx2));
		edges_.insert(SimEdge(idx2, idx0));
		edges_.insert(SimEdge(idx0, idx1));
	}
	buildHeap();
}

void Simplify::buildHeap() {
	heap_.clear();
	for (const auto &edge : edges_)
		pushHeap(edge);
}

sim::SimVertAdjustResult Simplify::calcAdjustEdgeResult(const SimEdge &edge) const {
	return {};
}

float Simplify::calcEdgeLengthSqr(const SimEdge &edge) const {
	const auto &p1 = vertices_[edge.start].position;
	const auto &p2 = vertices_[edge.last].position;
	return length(Math::Vector3(p1) - Math::Vector3(p2));
}

void Simplify::pushHeap(const SimEdge &edge) {
	if (calcEdgeLengthSqr(edge) < thresholdSqr_)
		return;
	auto &&[cost, point] = calcAdjustEdgeResult(edge);
	heap_.insert(CostRecord(cost, edge));
}

sim::CostRecord Simplify::popHeap() {
	auto iter = heap_.begin();
	auto result = *iter;
	heap_.erase(iter);
	return result;
}

}
