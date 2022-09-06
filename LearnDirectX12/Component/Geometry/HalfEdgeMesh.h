#pragma once
#include <array>
#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <map>
#include "Math/MathStd.hpp"
#include "GeometryGenerator.h"

namespace HalfEdge {

struct HEFace;
struct HEEdge;
struct HEVertex;
struct HEMesh;
using com::uint32;

struct HEVertex {
	Math::float3	position;
	Math::float2  texcoord;
	uint32  index;
};

struct HEEdge {
	HEVertex *start = nullptr;
	HEVertex *last = nullptr;
	HEFace	 *face = nullptr;
	bool	 isBoundary = false;
};

using HEEdgeKey = std::pair<const HEVertex *, const HEVertex *>;
struct HEEdgeKeyHasher {
	std::size_t operator()(const HEEdgeKey &key) const noexcept;
};

struct HEFace {
	std::array<HEEdge *, 3> edges;
};


struct HEMesh {
	std::vector<std::unique_ptr<HEVertex>>	verts;
	std::vector<std::unique_ptr<HEFace>>	faces;
	std::vector<std::unique_ptr<HEEdge>>	halfEdges;
	std::unordered_map<const HEVertex *, std::vector<HEFace *>>	vert2Face;
private:
	struct EdgeRefInfo {
		int		refCount = 0;
		HEEdge *halfEdge = nullptr;
	};
	mutable bool dirty = true;
	mutable std::unordered_map<HEEdgeKey, EdgeRefInfo, HEEdgeKeyHasher> edgeRefInfo;
	void updateVertBoundaryInfo() const;
public:
	HEMesh() = default;
	HEMesh(const com::MeshData &mesh);
	HEMesh(const std::vector<HEVertex> &vertices, std::vector<com::uint32> &indices);
	HEMesh(const HEMesh &other) = delete;
	~HEMesh() = default;

	void foreachFace(const std::function<void(const HEMesh *pSelf, const HEFace *pFace)> &callback) const;

	void foreachFace(const HEVertex *pVert, const std::function<void(HEFace *)> &callback) const;
	void foreachNeighborsVerts(const HEVertex *pVert, const std::function<void(HEVertex *)> &callback) const;
	void foreachNeighborsHalfVerts(const HEVertex *pVert, const std::function<void(HEVertex *)> &callback) const;
	void foreachNeighborsEdges(const HEVertex *pVert, const std::function<void(HEEdge *)> &callback) const;
	void foreachNeighborsHalfEdges(const HEVertex *pVert, const std::function<void(HEEdge *)> &callback) const;

	std::vector<HEFace *>	getFace(const HEVertex *vert) const;
	std::vector<HEVertex *> getNeighborsVerts(const HEVertex *vert) const;
	std::vector<HEVertex *> getNeighborsHalfVerts(const HEVertex *vert) const;
	std::vector<HEEdge *>	getNeighborsEdges(const HEVertex *vert) const;
	std::vector<HEEdge *>	getNeighborsHalfEdges(const HEVertex *vert) const;

	HEVertex *getVertex(size_t idx) const;

	bool isBoundaryVert(const HEVertex *pVert) const;
	bool isBoundaryEdge(const HEVertex *pVert1, const HEVertex *pVert2) const;
	bool isBoundaryEdge(const HEEdge *pEdge) const;
public:
	HEVertex *insertVertex(const Math::float3 &position, const Math::float2 &texcoord);
	HEEdge *insertEdge(HEVertex *v1, HEVertex *v2);
	HEFace *insertFace(const std::array<com::uint32, 3> &indices);
	friend void swap(HEMesh &lhs, HEMesh &rhs);
};

}
