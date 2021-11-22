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

struct Face;
struct Edge;
struct Vertex;
struct Mesh;


struct Vertex {
	float3	position;
	float2  texcoord;
	size_t  index;
};

struct Edge {
	Vertex *start;
	Vertex *last;
	Face *face;
};

struct Face {
	std::array<Edge *, 3> edges;
};


struct Mesh {
	std::vector<std::unique_ptr<Vertex>> verts;
	std::vector<std::unique_ptr<Face>> faces;
	std::vector<std::unique_ptr<Edge>> edges;
	std::unordered_map<Vertex *, std::vector<Edge *>> edgeMap;
public:
	Mesh(const com::MeshData &mesh);
	void foreachFace(const std::function<void(Face *)> &callback) const;
	std::vector<Face *> getFaceFromVertex(Vertex *vert) const;
	std::vector<Vertex *> getVertFromVertex(Vertex *vert) const;
	std::vector<Edge *> getEdgesFromVertex(Vertex *vert) const;
public:
	Vertex *insertVertex(const float3 &position, const float2 &texcoord);
	Edge *insertEdge(Vertex *v1, Vertex *v2);
};

}