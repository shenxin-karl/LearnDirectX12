#pragma once
#include "Math/MathHelper.h"

namespace com {


struct Box3D {
	Math::float3 min;
	Math::float3 max;
};

struct Box2D {
	Math::float2 min;
	Math::float2 max;
};

using AABB = Box3D;
using Rect = Box2D;

}