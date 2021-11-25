#include <iostream>
#include "Geometry/GeometryGenerator.h"
#include "Geometry/HalfEdgeMesh.h"

using namespace com;
using namespace vec;
using namespace HalfEdge;

void halfEdgeTest() {
	std::vector<Vertex> vertices = {
		Vertex{ float3(-1, 0, 0), float2(0, 0) },
		Vertex{ float3(0, 1, 0), float2(0.5, 1) },
		Vertex{ float3(1, 0, 0), float2(1, 0) },
	};
	std::vector<uint32> indices = {
		0, 1, 2,
	};
	MeshData mesh = { std::move(vertices), std::move(indices) };
	HEMesh hemesh(mesh);
	hemesh.foreachFace([&](const HEFace *face) {
		for (auto *pEdge : face->edges) {
			std::cout << "current vertex: " << pEdge->start->index << std::endl;
			std::vector<HEVertex *> nearHalfVertex = hemesh.getHalfVertsFromVertex(pEdge->start);
			std::cout << "----------------------- halfVerts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearHalfVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- halfVerts end -----------------------" << std::endl;
			std::vector<HEVertex *> nearVertex = hemesh.getVertsFromVertex(pEdge->start);
			std::cout << "----------------------- Verts begin -----------------------" << std::endl;
			for (HEVertex *pVert : nearVertex)
				std::cout << "index: " << pVert->index << std::endl;
			std::cout << "----------------------- Verts end -----------------------" << std::endl;
			std::vector<HEEdge *> nearHalfEdge = hemesh.getHalfEdgesFromVertex(pEdge->start);
			std::cout << "----------------------- HalfEdge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearHalfEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- HalfEdge end -----------------------" << std::endl;

			std::vector<HEEdge *> nearEdge = hemesh.getEdgesFromVertex(pEdge->start);
			std::cout << "----------------------- Edge begin -----------------------" << std::endl;
			for (HEEdge *pEdge : nearEdge) {
				std::cout << "start: " << pEdge->start->index << ", " 
					<< "last: " << pEdge->last->index << std::endl;
			}
			std::cout << "----------------------- Edge end -----------------------" << std::endl;
			std::cout << std::endl << std::endl;
		}
	});
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
	MeshData mesh = gen.createBox(10, 10, 100, 1);
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

int main() {
	halfEdgeTest();
	saveObjTest();
	createBoxTest();
	createCylinderTest();
	return 0;
}