#include "GeometryGenerator.h"
#include "Math/MathHelper.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

namespace com {

bool MeshData::save(const std::string &path) const {
	if (indices.empty())
		return false;

	std::fstream fout(path, std::ios::out);
	if (!fout.is_open())
		return false;

	std::stringstream posBuf;
	std::stringstream texBuf;
	std::stringstream norBuf;
	std::stringstream idxBuf;
	for (const Vertex &vertex : vertices) {
		const float3 &position = vertex.position;
		const float3 &normal = vertex.normal;
		const float2 &texcoord = vertex.texcoord;
		posBuf << "v " << position.x 
			   << " "  << position.y 
			   << " "  << position.z 
			   << '\n';
		norBuf << "vn " << normal.x
			   << " "   << normal.y
			   << " "   << normal.z
			   << '\n';
		texBuf << "vt " << texcoord.x
			   << " "   << texcoord.y
			   << " "   << 0.0f
			   << '\n';
	}
	for (uint32 i = 0; i < indices.size()-2; i += 3) {
		uint32 idx0 = indices[i + static_cast<size_t>(0)] + 1;
		uint32 idx1 = indices[i + static_cast<size_t>(1)] + 1;
		uint32 idx2 = indices[i + static_cast<size_t>(2)] + 1;
		idxBuf << "f " << idx0 << "/" << idx0 << "/" << idx0
			   << " "  << idx1 << "/" << idx1 << "/" << idx1
			   << " "  << idx2 << "/" << idx2 << "/" << idx2
			   << "\n";
	}

	fout << posBuf.rdbuf() << std::endl;
	fout << texBuf.rdbuf() << std::endl;
	fout << norBuf.rdbuf() << std::endl;
	fout << idxBuf.rdbuf() << std::endl;
	fout.close();
	return true;
}

bool GometryGenerator::generateNormal(MeshData &mesh) const {
	if (mesh.indices.size() < 3u) {
		std::cerr << "mesh.indices.size() < 3u" << std::endl;
		return false;
	}

	std::vector<float3> normals(mesh.vertices.size(), float3(0));
	for (uint32 i = 0; i < mesh.indices.size(); i += 3) {
		uint32 idx0 = mesh.indices[i + size_t(0)];
		uint32 idx1 = mesh.indices[i + size_t(1)];
		uint32 idx2 = mesh.indices[i + size_t(2)];
		const float3 &p0 = mesh.vertices[idx0].position;
		const float3 &p1 = mesh.vertices[idx1].position;
		const float3 &p2 = mesh.vertices[idx2].position;
		const float3 E1 = p1 - p0;
		const float3 E2 = p2 - p0;
		const float3 normal = cross(E1, E2);
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

bool GometryGenerator::generateTangent(MeshData &mesh) const {
	if (mesh.indices.size() < 3)
		return false;

	std::vector<float3> tangents(mesh.vertices.size(), float3(0));
	for (uint32 i = 0; i < mesh.indices.size()-2; i += 3) {
		uint32 idx0 = mesh.indices[i + static_cast<size_t>(0)];
		uint32 idx1 = mesh.indices[i + static_cast<size_t>(1)];
		uint32 idx2 = mesh.indices[i + static_cast<size_t>(2)];
		const Vertex &v0 = mesh.vertices[idx0];
		const Vertex &v1 = mesh.vertices[idx1];
		const Vertex &v2 = mesh.vertices[idx2];
		float3 E1 = v1.position - v0.position;
		float3 E2 = v2.position - v0.position;
		float t1 = v1.texcoord.y - v0.texcoord.y;
		float t2 = v2.texcoord.y - v0.texcoord.y;
		float3 tangent = (t2 * E1) - (t1 * E2);
		tangents[idx0] += tangent;
		tangents[idx1] += tangent;
		tangents[idx2] += tangent;
	}

	for (uint32 i = 0; i < tangents.size(); ++i) {
		Vertex &v = mesh.vertices[i];
		float3 t = tangents[i];
		t -= v.normal * dot(v.normal, t);		// 正交修正
		t = normalize(t);
		v.tangent = t;
	}

	return true;
}

bool GometryGenerator::generateTangentAndNormal(MeshData &mesh) const {
	if (mesh.indices.size() < 3)
		return false;

	std::vector<float3> normals(mesh.vertices.size(), float3(0));
	std::vector<float3> tangents(mesh.vertices.size(), float3(0));
	for (uint32 i = 0; i < mesh.indices.size()-2; i += 3) {
		uint32 idx0 = mesh.indices[i + static_cast<size_t>(0)];
		uint32 idx1 = mesh.indices[i + static_cast<size_t>(1)];
		uint32 idx2 = mesh.indices[i + static_cast<size_t>(2)];
		const Vertex &v0 = mesh.vertices[idx0];
		const Vertex &v1 = mesh.vertices[idx1];
		const Vertex &v2 = mesh.vertices[idx2];
		float3 E1 = v1.position - v0.position;
		float3 E2 = v2.position - v0.position;
		float3 normal = cross(E1, E2);
		float t1 = v1.texcoord.y - v0.texcoord.y;
		float t2 = v2.texcoord.y - v0.texcoord.y;
		float3 tangent = (t2 * E1) - (t1 * E2);
		for (uint32 j = i; j < i+3; ++j) {
			uint32 index = mesh.indices[j];
			normals[index] += normal;
			tangents[index] += tangent;
		}
	}

	for (uint32 i = 0; i < tangents.size(); ++i) {
		Vertex &v = mesh.vertices[i];
		float3 n = normalize(normals[i]);
		float3 t = tangents[i];
		t -= n * dot(n, t);		// 正交修正
		t = normalize(t);
		v.normal = n;
		v.tangent = t;
	}
	return true;
}

MeshData GometryGenerator::createCylinder(
	float bottomRadius, 
	float topRadius, 
	float height, 
	uint32 stackCount, 
	uint32 sliceCount
) const {
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
			vertices.push_back(vertex);
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
	// generate top
	if (topRadius > 0.f)
	{
		float topHeight = +0.5f * height;
		uint32 centerIdx = static_cast<uint32>(vertices.size());
		vertices.push_back(Vertex{ float3(0, topHeight, 0), float2(0.5f, 0.5f) });
		uint32 baseIdx = static_cast<uint32>(vertices.size());
		for (uint32 i = 0; i < sliceCount+1; ++i) {
			float radian = i * delta;
			float x = std::cos(radian) * topRadius;
			float z = std::sin(radian) * topRadius;
			float y = topHeight;
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;
			vertices.push_back(Vertex{ float3(x, y, z), float2(u, v) });
		}
		for (uint32 i = 0; i < sliceCount; ++i) {
			indices.push_back(baseIdx + i);
			indices.push_back(centerIdx);
			indices.push_back(baseIdx + i + 1);
		}
	}
	// generate bottom
	if (bottomRadius > 0.f) {
		float bottomHeight = -0.5f * height;
		uint32 centerIdx = static_cast<uint32>(vertices.size());
		vertices.push_back(Vertex{ float3(0, bottomHeight, 0), float2(0.5f, 0.5f) });
		uint32 baseIdx = static_cast<uint32>(vertices.size());
		for (uint32 i = 0; i < sliceCount+1; ++i) {
			float radian = i * delta;
			float x = std::cos(radian) * bottomRadius;
			float z = std::sin(radian) * bottomRadius;
			float y = bottomHeight;
			float u = x / height + 0.5f;
			float v = z / height + 0.5f;
			vertices.push_back(Vertex{ float3(x, y, z), float2(u, v) });
		}
		for (uint32 i = 0; i < sliceCount; ++i) {
			indices.push_back(baseIdx + i + 1);
			indices.push_back(centerIdx);
			indices.push_back(baseIdx + i);
		}
	}
	MeshData mesh = { std::move(vertices), std::move(indices) };
	generateTangentAndNormal(mesh);
	return mesh;
}

MeshData GometryGenerator::createBox(float width, float height, float depth, uint32 numSubdivisions) const {
	float x = 0.5f * width;
	float y = 0.5f * height;
	float z = 0.5f * depth;
	std::vector<Vertex> vertices = {
		// front
		Vertex{ float3(-x, -y, +z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(-x, +y, +z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, +z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, -y, +z), float2(1, 0), float3(+0, +0, +1), },
		// right
		Vertex{ float3(+x, -y, +z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, +z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, -z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, -y, -z), float2(1, 0), float3(+0, +0, +1), },
		// top
		Vertex{ float3(-x, +y, +z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, +z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, -z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(-x, +y, -z), float2(1, 0), float3(+0, +0, +1), },
		// left
		Vertex{ float3(-x, -y, -z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(-x, +y, -z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(-x, +y, +z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(-x, -y, +z), float2(1, 0), float3(+0, +0, +1), },
		// bottom
		Vertex{ float3(-x, -y, -z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(+x, -y, -z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, -y, +z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(-x, -y, +z), float2(1, 0), float3(+0, +0, +1), },
		// back
		Vertex{ float3(-x, +y, -z), float2(0, 0), float3(+0, +0, +1), },
		Vertex{ float3(-x, -y, -z), float2(0, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, -y, -z), float2(1, 1), float3(+0, +0, +1), },
		Vertex{ float3(+x, +y, -z), float2(1, 0), float3(+0, +0, +1), },
	};
	std::vector<uint32> indices = {
		// front
		0, 1, 2,
		0, 2, 3, 
		// right
		4, 5, 6,
		4, 6, 7,
		// top
		8, 9, 10,
		8, 10, 11,
		// left
		12, 13, 14,
		12, 14, 15,
		// bottom
		16, 17, 18,
		16, 18, 19,
		// back
		20, 21, 22,
		20, 22, 23,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	// todo 
	//for (uint32 i = 0; i < numSubdivisions-1; ++i)
	//	loopSubdivision(mesh);
	generateTangentAndNormal(mesh);
	return mesh;
}

MeshData GometryGenerator::createSphere(float radius, uint32 numSubdivisions) const {
	constexpr float x = 0.525731f;
	constexpr float y = 0.f;
	constexpr float z = 0.850651f;

	float3 pos[12] = {
		float3(-x, +y, +z), float3(+x, +y, +z),
		float3(-x, +y, -z), float3(+x, +y, -z),
		float3(+y, +z, +x), float3(+y, +z, -x),
		float3(+y, -z, +x), float3(+y, -z, -x),
		float3(+z, +x, +y), float3(-z, +x, +y),
		float3(+z, -x, +y), float3(-z, -x, +y),
	};

	uint32 k[60] = {
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7,
	};

	std::vector<Vertex> vertices;
	std::vector<uint32> indices(std::begin(k), std::end(k));
	vertices.reserve(12);
	
	for (const auto &position : pos) {
		float3 n = normalize(position);
		float3 point = n * radius;
		float u = std::atan2(n.z, n.x);
		float v = std::asin(n.y);
		vertices.push_back(Vertex{ point, float2(u, v) });
	}

	MeshData mesh = { std::move(vertices), std::move(indices) };
	return mesh;
}

MeshData GometryGenerator::createGrid(float width, float depth, uint32 m, uint32 n) const {
	std::vector<Vertex> vertices;
	float left = -0.5f * width;
	float right = +0.5f * width;
	float front = +0.5f * depth;
	float back = -0.5f * depth;
	for (uint32 i = 0; i < m+1; ++i) {
		for (uint32 j = 0; j < n+1; ++j) {
			float u = float(i) / float(m);
			float v = float(j) / float(n);
			float x = MathHelper::lerp(left, right, u);
			float z = MathHelper::lerp(front, back, v);
			vertices.push_back(Vertex{ float3(x, 0, z), float2(u, v) });
		}
	}

	std::vector<uint32> indices;
	uint32 sliceSize = n+1;
	for (uint32 i = 0; i < m; ++i) {
		for (uint32 j = 0; j < n; ++j) {
			indices.push_back((i+0) * sliceSize + (j+0));
			indices.push_back((i+0) * sliceSize + (j+1));
			indices.push_back((i+1) * sliceSize + (j+0));
			indices.push_back((i+0) * sliceSize + (j+1));
			indices.push_back((i+1) * sliceSize + (j+0));
			indices.push_back((i+1) * sliceSize + (j+1));
		}
	}
	MeshData mesh = { std::move(vertices), std::move(indices) };
	generateTangentAndNormal(mesh);
	return mesh;
}

MeshData GometryGenerator::createQuad(float x, float y, float w, float h, float depth) const {
	std::vector<Vertex> vertices = {
		Vertex{ float3(x, y, depth),		float2(0, 0) },
		Vertex{ float3(x, y+h, depth),		float2(0, 1) },
		Vertex{ float3(x+w, y+h, depth),	float2(1, 1) },
		Vertex{ float3(x+w, y, depth),		float2(0, 1) },
	};
	std::vector<uint32> indices = {
		0, 1, 2,
		0, 2, 3,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	generateTangentAndNormal(mesh);
	return mesh;
}

void GometryGenerator::loopSubdivision(MeshData &mesh) const {

}

void GometryGenerator::simplify(MeshData &mesh, float reserve) {

}

}