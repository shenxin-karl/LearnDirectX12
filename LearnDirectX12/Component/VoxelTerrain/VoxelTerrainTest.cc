#include "SurfaceNet.h"
#include "Geometry/GeometryGenerator.h"

using namespace Math;

int main() {
	float radius = 5.f;
	com::Box3D box = {
		float3(-radius),
		float3(+radius + 1.f),
	};
	auto implicitFunction = [=, radiusSqr = (radius * radius)](int x, int y, int z) {
		float3 point = float3(x, y, z) + box.min;
		return dot(point, point) - radiusSqr;
	};

	auto mesh = voxel::surfaceNet(implicitFunction, box, 0.f);
	mesh.savePTS("SurfaceNet.pts");
	com::GometryGenerator gen;
	gen.generateNormal(mesh);
	mesh.save("SurfaceNet.obj");
}