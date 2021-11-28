#include <iostream>
#include "Geometry/GeometryGenerator.h"
#include "Geometry/HalfEdgeMesh.h"
#include "Geometry/LoopSubdivision.h"
#include <unordered_map>
#include <unordered_set>

using namespace com;
using namespace vec;
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
		0, 2, 3,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	HEMesh hemesh(mesh);
	hemesh.foreachFace([](const HEMesh *pMesh, const HEFace *face) {
		for (auto *pEdge : face->edges) {
			std::cout << "current vertex: " << pEdge->start->index << std::endl;
			std::unordered_set<HEVertex *> nearHalfVertex = pMesh->getHalfVertsFromVertex(pEdge->start);
			std::cout << "----------------------- halfVerts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearHalfVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- halfVerts end -----------------------" << std::endl;
			std::unordered_set<HEVertex *> nearVertex = pMesh->getVertsFromVertex(pEdge->start);
			std::cout << "----------------------- Verts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- Verts end -----------------------" << std::endl;
			std::unordered_set<HEEdge *> nearHalfEdge = pMesh->getHalfEdgesFromVertex(pEdge->start);
			std::cout << "----------------------- HalfEdge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearHalfEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- HalfEdge end -----------------------" << std::endl;

			std::unordered_set<HEEdge *> nearEdge = pMesh->getEdgesFromVertex(pEdge->start);
			std::cout << "----------------------- Edge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- Edge end -----------------------" << std::endl;
			std::cout << std::endl << std::endl;
		}
	});

	std::cout << "union vert 0 2" << std::endl;
	auto unionVert = hemesh.getUnionVert(hemesh.getVertex(0), hemesh.getVertex(2));
	for (auto *pVert : unionVert)
		std::cout << "index: " << pVert->index << std::endl;
	std::cout << "union vert 0 2 end" << std::endl;

	std::cout << "union vert 0 3" << std::endl;
	unionVert = hemesh.getUnionVert(hemesh.getVertex(0), hemesh.getVertex(3));
	for (auto *pVert : unionVert)
		std::cout << "index: " << pVert->index << std::endl;
	std::cout << "union vert 0 3 end" << std::endl;
	std::cout << "union vert 2 1" << std::endl;
	unionVert = hemesh.getUnionVert(hemesh.getVertex(2), hemesh.getVertex(1));
	for (auto *pVert : unionVert)
		std::cout << "index: " << pVert->index << std::endl;
	std::cout << "union vert 2 1 end" << std::endl;
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
	Vertex{ float3(0, 0, 0), float2(0) },
	Vertex{ float3(0, 1, 0), float2(0) },
	Vertex{ float3(1, 1, 0), float2(0) },
	Vertex{ float3(1, 0, 0), float2(0) },
	};
	std::vector<uint32> indices = {
		0, 1, 2,
		0, 2, 3,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	//com::GometryGenerator gen;
	//MeshData mesh = gen.createCylinder(10, 10, 10, 10, 10);
	com::LoopSubdivision subdivision;
	for (int i = 0; i < 3; ++i)
		mesh = subdivision.subdivide2MeshData(mesh);
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

int main() {
	//halfEdgeTest();
	//saveObjTest();
	//createBoxTest();
	//createCylinderTest();
	//loopSubdivisionTest();
	//loopBetaTest();
	//createShapeTest();
	return 0;
}