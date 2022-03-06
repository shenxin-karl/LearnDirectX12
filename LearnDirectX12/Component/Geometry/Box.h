#pragma once
#include "Math/MathHelper.h"

namespace com {

using namespace Math;

struct Box3D {
	float3 min;
	float3 max;
};

struct Box2D {
	float2 min;
	float2 max;
};

using AABB = Box3D;
using Rect = Box2D;

}