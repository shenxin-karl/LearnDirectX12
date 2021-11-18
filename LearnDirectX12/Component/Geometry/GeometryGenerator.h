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
};

class GometryGenerator {
public:
	static bool generateNormal(MeshData &mesh);
	static bool generateTangent(MeshData &mesh);
	static bool generateTangentAndNormal(MeshData &mesh);

	static MeshData createCylinkder(
		float bottomRadius, 
		float topRadius, 
		float height, 
		uint32 stackCount, 
		uint32 sliceCount
	);
};

}