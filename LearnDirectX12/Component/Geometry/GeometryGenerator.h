#pragma once
#include "Math/VectorHelper.h"
#include <variant>
#include <vector>

namespace com {
using namespace vec;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;

struct Vertex {
	float3	position;
	float2  texcoord;
	float3	normal;
	float3	tangent;
};

struct MeshData {
	std::vector<Vertex> vertices;
	std::vector<uint32>	indices;
public:
	bool save(const std::string &path) const;
};

class GometryGenerator {
public:
	bool generateNormal(MeshData &mesh) const;
	bool generateTangent(MeshData &mesh) const;
	bool generateTangentAndNormal(MeshData &mesh) const;

	MeshData createCylinder(
		float bottomRadius, 
		float topRadius, 
		float height, 
		uint32 stackCount, 
		uint32 sliceCount
	) const;

	MeshData createBox(float width, float height, float depth, uint32 numSubdivisions) const;

	MeshData createSphere(float radius, uint32 numSubdivisions) const;

	MeshData createGrid(float width, float depth, uint32 m, uint32 n) const;

	MeshData createQuad(float x, float y, float w, float h, float depth) const;

	void loopSubdivision(MeshData &mesh) const;	

	void simplify(MeshData &mesh, float reserve);
};

}