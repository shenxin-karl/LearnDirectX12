#pragma once
#include <DirectXMath.h>
#include "Math/MatrixHelper.h"

#define FORCEINLINE __forceinline


namespace Math {

namespace DX = DirectX;
namespace XM = DirectX;

class MathHelper {
public:
	static float4x4 identity4x4();
	static float4x3 identity4x3();
	static DX::XMFLOAT3X4 identity3x4();
	static float3x3 identity3x3();

	template<typename T>
	static T lerp(const T &lhs, const T &rhs, float t) {
		return lhs + (rhs - lhs) * t;
	}
};

using namespace XM;

}



