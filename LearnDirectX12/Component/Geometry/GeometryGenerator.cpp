#include "GeometryGenerator.h"
#include "Math/MathHelper.h"
#include <cmath>
#include <iostream>

namespace com {

bool GometryGenerator::generateNormal(MeshData &mesh) {
	if (mesh.indices.size() < 3u) {
		std::cerr << "mesh.indices.size() < 3u" << std::endl;
		return false;
	}

	std::vector<float3> normals(mesh.vertices.size(), float3(0));
	for (uint32 i = 0; i < mesh.indices.size(); i += 3) {
		uint32 idx0 = mesh.indices[i+0u];
		uint32 idx1 = mesh.indices[i+1u];
		uint32 idx2 = mesh.indices[i+2u];
		const float3 &p0 = mesh.vertices[idx0].position;
		const float3 &p1 = mesh.vertices[idx1].position;
		const float3 &p2 = mesh.vertices[idx2].position;
		const float3 e1 = p1 - p0;
		const float3 e2 = p2 - p0;
		const float3 normal = cross(e1, e2);
		normals[idx0] += normal;
		normals[idx1] += normal;
		normals[idx2] += normal;
	}
	for (uint32 i = 0; i < mesh.vertices.size(); ++i) {
		float3 normal = normalize(normals[i]);
		mesh.vertices[i].normal = normal;
	}
	return true;
}

MeshData GometryGenerator::createCylinkder(
	float bottomRadius, 
	float topRadius, 
	float height, 
	uint32 stackCount, 
	uint32 sliceCount
) {
	std::vector<Vertex> vertices;
	std::vector<uint32> indices;
	uint32 ringVertexCount = sliceCount + 1;
	float delta = DX::XM_2PI / sliceCount;
	for (uint32 i = 0; i < stackCount+1; ++i) {
		for (uint32 j = 0; j < ringVertexCount; ++j) {
			float ratio = float(i) / float(stackCount);
			float radius = MathHelper::lerp(bottomRadius, topRadius, ratio);
			float radian = j * delta;
			float x = std::cos(radian) * radius;
			float y = MathHelper::lerp(-0.5f*height, +0.5f*height, ratio);;
			float z = std::sin(radian) * radius;
			float u = ratio; 
			float v = float(j) / float(sliceCount);
			Vertex vertex;
			vertex.position = float3(x, y, z);
			vertex.texcoord = float2(u, v);
		}
	}
	for (uint32 i = 0; i < stackCount; ++i) {
		for (uint32 j = 0; j < sliceCount; ++j) {
			indices.push_back((i+0) * ringVertexCount + j+0);
			indices.push_back((i+1) * ringVertexCount + j+0);
			indices.push_back((i+1) * ringVertexCount + j+1);
			indices.push_back((i+1) * ringVertexCount + j+1);
			indices.push_back((i+0) * ringVertexCount + j+1);
			indices.push_back((i+0) * ringVertexCount + j+0);
		}
	}
	return { std::move(vertices), std::move(indices) };
}

}