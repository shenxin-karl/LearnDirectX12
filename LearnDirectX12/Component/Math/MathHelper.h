#pragma once
#include <DirectXMath.h>
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"

namespace Math {

namespace DX = DirectX;
namespace XM = DirectX;

class MathHelper {
public:
	static DX::XMFLOAT4X4 identity4x4();
	static DX::XMFLOAT4X3 identity4x3();
	static DX::XMFLOAT3X4 identity3x4();
	static DX::XMFLOAT3X3 identity3x3();
	template<typename T>
	static T lerp(const T &lhs, const T &rhs, float t) {
		return lhs + (rhs - lhs) * t;
	}
};

using namespace XM;

}



