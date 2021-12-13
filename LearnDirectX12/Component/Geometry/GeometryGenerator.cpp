#include "GeometryGenerator.h"
#include "Math/MathHelper.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <unordered_map>

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
		{ Vertex{ float3(-x, -y, -z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(-x, +y, -z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(+x, +y, -z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(+x, -y, -z), float2(1.0f, 1.0f), } },
		{ Vertex{ float3(-x, -y, +z), float2(1.0f, 1.0f), } },
		{ Vertex{ float3(+x, -y, +z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(+x, +y, +z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(-x, +y, +z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(-x, +y, -z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(-x, +y, +z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(+x, +y, +z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(+x, +y, -z), float2(1.0f, 1.0f), } },
		{ Vertex{ float3(-x, -y, -z), float2(1.0f, 1.0f), } },
		{ Vertex{ float3(+x, -y, -z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(+x, -y, +z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(-x, -y, +z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(-x, -y, +z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(-x, +y, +z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(-x, +y, -z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(-x, -y, -z), float2(1.0f, 1.0f), } },
		{ Vertex{ float3(+x, -y, -z), float2(0.0f, 1.0f), } },
		{ Vertex{ float3(+x, +y, -z), float2(0.0f, 0.0f), } },
		{ Vertex{ float3(+x, +y, +z), float2(1.0f, 0.0f), } },
		{ Vertex{ float3(+x, -y, +z), float2(1.0f, 1.0f), } },
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
	for (uint32 i = 0; i < numSubdivisions; ++i) {
		std::vector<uint32> newIndices;
		newIndices.swap(indices);
		indices.reserve(newIndices.size() * 4);
		for (size_t j = 0; j < newIndices.size()-2; j += 3) {
			uint32 idx0 = newIndices[j + 0];
			uint32 idx1 = newIndices[j + 1];
			uint32 idx2 = newIndices[j + 2];
			uint32 baseIdx = static_cast<uint32>(vertices.size());
			vertices.push_back(middleVertex(vertices[idx0], vertices[idx1]));
			vertices.push_back(middleVertex(vertices[idx1], vertices[idx2]));
			vertices.push_back(middleVertex(vertices[idx2], vertices[idx0]));
			indices.insert(indices.end(), { idx0, baseIdx+0, baseIdx+2 });
			indices.insert(indices.end(), { baseIdx+0, idx1, baseIdx+1 });
			indices.insert(indices.end(), { baseIdx+1, idx2, baseIdx+2 });
			indices.insert(indices.end(), { baseIdx+0, baseIdx+1, baseIdx+2 });
		}
	}

	MeshData mesh = { std::move(vertices), std::move(indices) };
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
	vertices.reserve(12);
	std::transform(std::begin(pos), std::end(pos), std::back_inserter(vertices), [](const float3 &position) {
		return Vertex{ position };
	});
	std::vector<uint32> indices(std::begin(k), std::end(k));
	
	for (uint32 i = 0; i < numSubdivisions; ++i) {
		std::vector<uint32> newIndices;
		newIndices.swap(indices);
		indices.reserve(newIndices.size() * 4);
		for (size_t j = 0; j < newIndices.size()-2; j += 3) {
			uint32 idx0 = newIndices[j + 0];
			uint32 idx1 = newIndices[j + 1];
			uint32 idx2 = newIndices[j + 2];
			uint32 baseIdx = static_cast<uint32>(vertices.size());
			vertices.push_back(middlePoint(vertices[idx0], vertices[idx1]));
			vertices.push_back(middlePoint(vertices[idx1], vertices[idx2]));
			vertices.push_back(middlePoint(vertices[idx2], vertices[idx0]));
			indices.insert(indices.end(), { idx0, baseIdx+0, baseIdx+2 });
			indices.insert(indices.end(), { baseIdx+0, idx1, baseIdx+1 });
			indices.insert(indices.end(), { baseIdx+1, idx2, baseIdx+2 });
			indices.insert(indices.end(), { baseIdx+0, baseIdx+1, baseIdx+2 });
		}
	}
	for (auto &vert : vertices) {
		float3 n = normalize(vert.position);
		float3 point = n * radius;
		float u = std::atan2(n.z, n.x);
		float v = std::asin(n.y);
		vert.position = point;
		vert.texcoord = float2(u, v);
	}
	MeshData mesh = { std::move(vertices), std::move(indices) };
	generateTangentAndNormal(mesh);
	return mesh;
}

MeshData GometryGenerator::createGrid(float width, float depth, uint32 m, uint32 n) const {
	std::vector<Vertex> vertices;
	float left = -0.5f * width;
	float right = +0.5f * width;
	float far = +0.5f * depth;
	float near = -0.5f * depth;
	for (uint32 zi = 0; zi < n+1; ++zi) {
		for (uint32 xi = 0; xi < m+1; ++xi) {
			float u = float(xi) / float(m);
			float v = float(zi) / float(n);
			float x = MathHelper::lerp(left, right, u);
			float z = MathHelper::lerp(far, near, v);
			vertices.push_back(Vertex{ float3(x, 0, z), float2(u, v) });
		}
	}

	std::vector<uint32> indices;
	uint32 sliceSize = m+1;
	for (uint32 zi = 0; zi < n; ++zi) {
		for (uint32 xi = 0; xi < m; ++xi) {
			indices.push_back((zi+0) * sliceSize + (xi+0));
			indices.push_back((zi+0) * sliceSize + (xi+1));
			indices.push_back((zi+1) * sliceSize + (xi+0));
			indices.push_back((zi+1) * sliceSize + (xi+0));
			indices.push_back((zi+0) * sliceSize + (xi+1));
			indices.push_back((zi+1) * sliceSize + (xi+1));
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


struct VertexDataIndex {
	com::uint32 posIdx = 0;
	com::uint32 nrmIdx = 0;
	com::uint32 texIdx = 0;
};

struct VertexDataIndexHasher {
	std::size_t operator()(const VertexDataIndex &vert) const noexcept {
		return std::hash<com::uint32>{}(vert.posIdx) << 0 ^
			std::hash<com::uint32>{}(vert.nrmIdx) << 1 ^
			std::hash<com::uint32>{}(vert.texIdx) << 2;
	}
};

std::tuple<bool, MeshData> GometryGenerator::loadObjFile(const std::string &path) {
	std::fstream fin(path, std::ios::in);
	std::tuple<bool, MeshData> result(false, {});
	if (!fin.is_open())
		return result;

	std::vector<std::string> strPositions;
	std::vector<std::string> strTexcoords;
	std::vector<std::string> strNormals;
	std::vector<std::string> strFaces;

	std::string line;
	while (!fin.eof()) {
		getline(fin, line);
		if (line.compare(0, 1, "f"))
			strFaces.push_back(std::move(line));
		else if (line.compare(0, 1, "v"))
			strPositions.push_back(std::move(line));
		else if (line.compare(0, 2, "vt"))
			strTexcoords.push_back(std::move(line));
		else if (line.compare(0, 2, "vn"))
			strNormals.push_back(std::move(line));
	}

	std::vector<float3> positions(strPositions.size(), float3(0.f));
	std::vector<float3> normals(strNormals.size(), float3(0.f));
	std::vector<float2> texcoords(strTexcoords.size(), float2(0.f));
	for (std::size_t i = 0; i < strPositions.size(); ++i) {
		auto &pos = positions[i];
		(void)sscanf(strPositions[i].c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
	}
	for (std::size_t i = 0; i < strNormals.size(); ++i) {
		auto &nrm = normals[i];
		(void)scanf(strNormals[i].c_str(), "vn %f %f %f", &nrm.x, nrm.y, &nrm.z);
	}
	for (std::size_t i = 0; i < strTexcoords.size(); ++i) {
		auto &uv = texcoords[i];
		(void)scanf(strTexcoords[i].c_str(), "vt %f %f", &uv.x, uv.y);
	}

	strPositions.~vector();
	strNormals.~vector();
	strTexcoords.~vector();
	std::vector<Vertex> vertices;
	std::vector<com::uint32> indices;

	uint32 flag = 0x1;
	flag |= (texcoords.empty() ? 0 : 1) << 1;
	flag |= (normals.empty() ? 0 : 1) << 2;
	std::unordered_map<VertexDataIndex, std::size_t, VertexDataIndexHasher> record;
	for (const auto &line : strFaces) {
		switch (flag) {
		case 3:
			break;
		case 5:
			break;
		case 7:
		}
	}

	return result;
}

com::Vertex GometryGenerator::middlePoint(const Vertex &lhs, const Vertex &rhs) {
	return {
		MathHelper::lerp(lhs.position, rhs.position, 0.5f)
	};
}


com::Vertex GometryGenerator::middleVertex(const Vertex &lhs, const Vertex &rhs) {
	return {
		MathHelper::lerp(lhs.position, rhs.position, 0.5f),
		MathHelper::lerp(lhs.texcoord, rhs.texcoord, 0.5f),
	};
}

}