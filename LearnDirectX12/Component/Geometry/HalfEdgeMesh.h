#pragma once
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "Math/VectorHelper.h"
#include "GeometryGenerator.h"

namespace HalfEdge {
using namespace vec;

struct HEFace;
struct HEEdge;
struct HEVertex;
struct HEMesh;


struct HEVertex {
	float3	position;
	float2  texcoord;
	size_t  index;
};

struct HEEdge {
	HEVertex *start = nullptr;
	HEVertex *last = nullptr;
	HEFace *face = nullptr;
};

struct HEFace {
	std::array<HEEdge *, 3> edges;
};


struct HEMesh {
	std::vector<std::unique_ptr<HEVertex>> verts;
	std::vector<std::unique_ptr<HEFace>> faces;
	std::vector<std::unique_ptr<HEEdge>> edges;
	std::unordered_map<HEVertex *, std::vector<HEFace *>> faceMap;
public:
	HEMesh(const com::MeshData &mesh);
	HEMesh(const std::vector<HEVertex> &vertices, std::vector<com::uint32> &indices);
	HEMesh(const HEMesh &other);
	HEMesh(HEMesh &&other) = default;
	HEMesh &operator=(HEMesh &&other) = default;
	void foreachFace(const std::function<void(const HEFace *)> &callback) const;
	std::vector<HEFace *> getFaceFromVertex(HEVertex *vert) const;
	std::vector<HEVertex *> getVertsFromVertex(HEVertex *vert) const;
	std::vector<HEVertex *> getHalfVertsFromVertex(HEVertex *vert) const;
	std::vector<HEEdge *> getEdgesFromVertex(HEVertex *vert) const;
	std::vector<HEEdge *> getHalfEdgesFromVertex(HEVertex *vert) const;
	HEVertex *getVertex(size_t idx) const;
	size_t getVertexFaceCount(HEVertex *pFace) const;
	bool hasFace() const;
public:
	HEVertex *insertVertex(const float3 &position, const float2 &texcoord);
	HEEdge *insertEdge(HEVertex *v1, HEVertex *v2);
	HEFace *insertFace(const std::array<com::uint32, 3> &indices);
};

}