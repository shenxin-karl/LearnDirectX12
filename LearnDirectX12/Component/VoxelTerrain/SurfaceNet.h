#pragma once
#include "Math/MathHelper.h"
#include "Geometry/GeometryGenerator.h"
#include "Geometry/Box.h"
#include <vector>
#include <functional>
#include <array>

namespace voxel {

using namespace Math;
using uint16 = std::uint16_t;


struct int3 {
	int x;
	int y;
	int z;
public:
	operator float3 () const {
		return { float(x), float(y), float(z) };
	}
};

inline int3 operator+(const int3 &lhs, const int3 &rhs) {
	return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

struct Box {
	int3 min;
	int3 max;
};

com::MeshData surfaceNet(const std::function<float(int, int, int)> &implicitFunction, 
	const com::Box3D &box,			// 体素大小
	float isovalue = 0.f		// 等值面
);

}