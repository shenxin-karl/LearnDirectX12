#include <iostream>
#include "Geometry/GeometryGenerator.h"
#include "Geometry/HalfEdgeMesh.h"
#include "Geometry/LoopSubdivision.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <string>

using namespace com;
using namespace Math;
using namespace HalfEdge;

void halfEdgeTest() {
	/*
	*	1--------2
	*	| 	  *  |
	*	|  *	 |
	*	0--------3
	* 0->1->2
	*/

	std::vector<Vertex> vertices = {
		Vertex{ float3(0, 0, 0), float2(0) },
		Vertex{ float3(0, 1, 0), float2(0) },
		Vertex{ float3(1, 1, 0), float2(0) },
		Vertex{ float3(1, 0, 0), float2(0) },
	};
	std::vector<uint32> indices = {
		0, 1, 2,
		//0, 2, 3,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	HEMesh hemesh(mesh);
	hemesh.foreachFace([](const HEMesh *pMesh, const HEFace *face) {
		for (auto *pEdge : face->edges) {
			std::cout << "current vertex: " << pEdge->start->index << std::endl;
			std::vector<HEVertex *> nearHalfVertex = pMesh->getNeighborsHalfVerts(pEdge->start);
			std::cout << "----------------------- halfVerts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearHalfVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- halfVerts end -----------------------" << std::endl;
			std::vector<HEVertex *> nearVertex = pMesh->getNeighborsVerts(pEdge->start);
			std::cout << "----------------------- Verts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- Verts end -----------------------" << std::endl;
			std::vector<HEEdge *> nearHalfEdge = pMesh->getNeighborsHalfEdges(pEdge->start);
			std::cout << "----------------------- HalfEdge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearHalfEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- HalfEdge end -----------------------" << std::endl;

			std::vector<HEEdge *> nearEdge = pMesh->getNeighborsEdges(pEdge->start);
			std::cout << "----------------------- Edge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- Edge end -----------------------" << std::endl;
			std::cout << std::endl << std::endl;
		}
	});

	//std::cout << "union vert 0 2" << std::endl;
	//auto unionVert = hemesh.getUnionVert(hemesh.getVertex(0), hemesh.getVertex(2));
	//for (auto *pVert : unionVert)
	//	std::cout << "index: " << pVert->index << std::endl;
	//std::cout << "union vert 0 2 end" << std::endl;

	//std::cout << "union vert 0 3" << std::endl;
	//unionVert = hemesh.getUnionVert(hemesh.getVertex(0), hemesh.getVertex(3));
	//for (auto *pVert : unionVert)
	//	std::cout << "index: " << pVert->index << std::endl;
	//std::cout << "union vert 0 3 end" << std::endl;
	//std::cout << "union vert 2 1" << std::endl;
	//unionVert = hemesh.getUnionVert(hemesh.getVertex(2), hemesh.getVertex(1));
	//for (auto *pVert : unionVert)
	//	std::cout << "index: " << pVert->index << std::endl;
	//std::cout << "union vert 2 1 end" << std::endl;
}

void saveObjTest() {
	std::vector<Vertex> vertice = {
		Vertex { float3(0, 0, 0), float2(0, 0), float3(0, 0, 1), float3(1, 0, 0) },
		Vertex { float3(0, 1, 0), float2(0, 1), float3(0, 0, 1), float3(1, 0, 0) },
		Vertex { float3(1, 1, 0), float2(1, 1), float3(0, 0, 1), float3(1, 0, 0) },
		Vertex { float3(1, 0, 0), float2(1, 0), float3(0, 0, 1), float3(1, 0, 0) },
	};

	std::vector<uint32> indices = {
		0, 1, 2,
		0, 2, 3,
	};

	MeshData mesh = { std::move(vertice), std::move(indices) };
	mesh.save("SaveObjTest.obj");
}

void createBoxTest() {
	com::GometryGenerator gen;
	MeshData mesh = gen.createBox(100, 100, 100, 3);
	mesh.save("CreateBoxTest.obj");
}

void createCylinderTest() {
	com::GometryGenerator gen;
	MeshData mesh = gen.createCylinder(10, 10, 10, 10, 10);
	mesh.save("CreateBoxTest1.obj");
	mesh = gen.createCylinder(0, 10, 10, 10, 10);
	mesh.save("CreateBoxTest2.obj");
	mesh = gen.createCylinder(10, 0, 10, 10, 10);
	mesh.save("CreateBoxTest3.obj");
	mesh = gen.createCylinder(10, 0, 10, 100, 100);
	mesh.save("CreateBoxTest4.obj");
}

void loopSubdivisionTest() {
	std::vector<Vertex> vertices = {
		Vertex{ float3(0.125, 0.125, 0), float2(0.f) },
		Vertex{ float3(0.125, 0.875, 0), float2(0.f) },
		Vertex{ float3(0.875, 0.875, 0), float2(0.f) },
		Vertex{ float3(0.875, 0.125, 0), float2(0.f) },
		Vertex{ float3(0, 0.5, 0), float2(0.f) },
		Vertex{ float3(0.5, 1, 0), float2(0.f) },
		Vertex{ float3(0.5, 0.5, 0), float2(0.f) },
		Vertex{ float3(1, 0.5, 0), float2(0.f) },
		Vertex{ float3(0.5, 0, 0), float2(0.f) },
	};
	std::vector<uint32> indices = {
		1-1, 5-1, 7-1,
		5-1, 2-1, 6-1,
		6-1, 3-1, 7-1,
		5-1, 6-1, 7-1,
		1-1, 7-1, 9-1,
		7-1, 3-1, 8-1,
		8-1, 4-1, 9-1,
		7-1, 8-1, 9-1,
	};
	//MeshData mesh = { std::move(vertices), std::move(indices) };
	com::GometryGenerator gen;
	MeshData mesh = gen.createBox(10, 10, 10, 0);
	//MeshData mesh = gen.loadObjFile("bunny.obj");
	loop::LoopSubdivision subdivision;
	mesh = subdivision.subdivision(mesh, 1);
	mesh.save("loopSubdivisionTest.obj");
}

void loopBetaTest() {
	for (int i = 2; i < 20; ++i) {
		float n = static_cast<float>(i);
		float _1_div_n = 1.f / n;
		constexpr float _5_div_8 = 5.f / 8.f;
		constexpr float _3_div_8 = 3.f / 8.f;
		constexpr float _1_div_4 = 1.f / 4.f;
		float alpha = _3_div_8 + _1_div_4 * std::cos(DX::XMConvertToRadians(360.f / n));
		float beta = _1_div_n * (_5_div_8 - alpha * alpha);
		std::cout << "n: " << n << "\tv0: " << 1.f - n * beta << std::endl;
	}
}

void createShapeTest() {
	com::GometryGenerator gen;
	auto mesh = gen.createSphere(10, 7);
	mesh.save("createShapeTest.obj");
}

void createGridTest() {
	com::GometryGenerator gen;
	auto mesh = gen.createGrid(20.0f, 30.0f, 60, 40);
	mesh.save("createGridTest.obj");
}

void loadObject() {
	com::GometryGenerator gen;
	MeshData meshData = gen.loadObjFile("createGridTest.obj");
	meshData.save("loadObject.obj");
}

bool convertTxtModelToObjFile(const std::string &path, const std::string &outputPath) {
	std::fstream fin(path, std::ios::in);
	if (!fin.is_open()) {
		std::cerr << "can't open the file: " << path << std::endl;
		return false;
	}

	std::string line;
	std::getline(fin, line);
	uint32 vertCount = 0;
	if (sscanf_s(line.c_str(), "VertexCount: %u", &vertCount) != 1 && vertCount <= 0)
		return false;

	std::getline(fin, line);
	uint32 triCount = 0;
	if (sscanf_s(line.c_str(), "TriangleCount: %u", &triCount) && triCount <= 0)
		return false;

	std::vector<Vertex> vertices(vertCount, { float3(0.f), float2(0.f), float3(0.f), float3(0.f) });
	std::vector<uint32> indices;
	indices.reserve(static_cast<size_t>(triCount) * 3);
	while (!fin.eof()) {
		std::getline(fin, line);
		if (line.compare(0, 10, "VertexList") == 0) {
			std::getline(fin, line);
			for (uint32 i = 0; i < vertCount; ++i) {
				std::getline(fin, line);
				auto &vert = vertices[i];
				int ret = sscanf_s(line.c_str(), "\t%f %f %f %f %f %f",
					&vert.position.x, &vert.position.y, &vert.position.z,
					&vert.normal.x, &vert.normal.y, &vert.normal.z
				);
				if (ret != 6)
					return false;
			}
		} else if (line.compare(0, 12, "TriangleList") == 0) {
			std::getline(fin, line);
			for (uint32 i = 0; i < triCount; ++i) {
				std::getline(fin, line);
				uint32 idx0, idx1, idx2;
				if (sscanf_s(line.c_str(), "\t%u %u %u", &idx0, &idx1, &idx2) != 3)
					return false;
				indices.push_back(idx0);
				indices.push_back(idx1);
				indices.push_back(idx2);
			}
		}
	}

	MeshData meshData = { std::move(vertices), std::move(indices) };
	meshData.save(outputPath);
	return true;
}

int main() {
	//halfEdgeTest();
	//saveObjTest();
	//createBoxTest();
	//createCylinderTest();
	loopSubdivisionTest();
	//loopBetaTest();
	//createShapeTest();
	//createGridTest();
	//loadObject();
	//convertTxtModelToObjFile("skull.txt", "skull.obj");
	return 0;
}