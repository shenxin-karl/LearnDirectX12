#pragma once
#include "Math/MathHelper.h"
#include "Math/VectorHelper.h"
//#include "Math/MatrixHelper.h"

namespace d3dUlti {
using namespace vec;

struct Light {
	float3	strength	= float3(0.5f);				// 光的辐射强度
	float	falloffStar = 1.0f;						// 点光源/聚光灯使用
	float3	direction	= float3(0.f, -1.f, 0.f);	// 方向光源使用
	float	falloffEnd	= 10.f;						// 点光源/聚光灯使用
	float3	position	= float3(0.f);				// 点光源/聚光灯使用
	float	spotPower	= 64.f;						// 聚光灯使用
};

}