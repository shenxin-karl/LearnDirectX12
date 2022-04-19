#include <DirectXMath.h>
#include "Swizzle.hpp"
#include "VectorHelper.h"

namespace Math {

namespace DX = DirectX;

class alignas(sizeof(DX::XMVECTOR)) Vector3 {
public:
	FORCEINLINE Vector3() = default;
	FORCEINLINE Vector3(const Vector3 &) noexcept = default;
	FORCEINLINE Vector3(DX::XMVECTOR vec) noexcept : vec(vec) {
	}
	FORCEINLINE Vector3(float x, float y, float z) noexcept {
		vec = DX::XMVectorSet(x, y, z, 0.0);
	}
	FORCEINLINE explicit Vector3(float val) noexcept {
		vec = DX::XMVectorSet(val, val, val, 0.0);
	}
	FORCEINLINE Vector3 &operator=(const Vector3 &other) noexcept {
		vec = other.vec;
	}
	FORCEINLINE float &operator[](size_t index) noexcept {
		assert(index < 3);
		return vec.m128_f32[index];
	}
	FORCEINLINE float operator[](size_t index) const noexcept {
		assert(index < 3);
		return vec.m128_f32[index];
	}
	FORCEINLINE DX::XMVECTOR *operator&() noexcept {
		return &vec;
	}
	FORCEINLINE const DX::XMVECTOR *operator&() const noexcept {
		return &vec;
	}
	FORCEINLINE operator DX::XMVECTOR &() noexcept {
		return vec;
	}
	FORCEINLINE operator const  DX::XMVECTOR &() const noexcept {
		return vec;
	}
	FORCEINLINE friend Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorAdd(lhs, rhs);
	}
	FORCEINLINE friend Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorSubtract(lhs, rhs);
	}
	FORCEINLINE friend Vector3 operator*(const Vector3 &lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorMultiply(lhs, rhs);
	}
	FORCEINLINE friend Vector3 operator/(const Vector3 &lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorDivide(lhs, rhs);
	}
	FORCEINLINE friend Vector3 operator+(float lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorAdd(Vector3(lhs), rhs);
	}
	FORCEINLINE friend Vector3 operator-(float lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorSubtract(Vector3(lhs), rhs);
	}
	FORCEINLINE friend Vector3 operator*(float lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorMultiply(Vector3(lhs), rhs);
	}
	FORCEINLINE friend Vector3 operator/(float lhs, const Vector3 &rhs) noexcept {
		return DX::XMVectorDivide(Vector3(lhs), rhs);
	}
	FORCEINLINE Vector3 &operator+=(const Vector3 &other) noexcept {
		*this = *this + other;
		return *this;
	}
	FORCEINLINE Vector3 &operator-=(const Vector3 &other) noexcept {
		*this = *this - other;
		return *this;
	}
	FORCEINLINE Vector3 &operator*=(const Vector3 &other) noexcept {
		*this = *this * other;
		return *this;
	}
	FORCEINLINE Vector3 &operator/=(const Vector3 &other) noexcept {
		*this = *this / other;
		return *this;
	}
	FORCEINLINE Vector3 &operator+=(float other) noexcept {
		*this = *this + Vector3(other);
		return *this;
	}
	FORCEINLINE Vector3 &operator-=(float other) noexcept {
		*this = *this - Vector3(other);
		return *this;
	}
	FORCEINLINE Vector3 &operator*=(float other) noexcept {
		*this = *this * Vector3(other);
		return *this;
	}
	FORCEINLINE Vector3 &operator/=(float other) noexcept {
		*this = *this / Vector3(other);
		return *this;
	}
	FORCEINLINE friend Vector3 cross(const Vector3 &lhs, const Vector3 &rhs) noexcept {
		return DX::XMVector3Cross(lhs, rhs);
	}
	FORCEINLINE friend Vector3 normalize(const Vector3 &vec) noexcept {
		return DX::XMVector3Normalize(vec);
	}
public:
	union {
		DX::XMVECTOR vec;
		Swizzle<float, true, 0>		x;
		Swizzle<float, true, 1>		y;
		Swizzle<float, true, 2>		z;

		Swizzle<float2, false, 0, 0>	xx;
		Swizzle<float2, true, 0, 1>		xy;
		Swizzle<float2, true, 0, 2>		xz;
		Swizzle<float2, true, 1, 0>		yx;
		Swizzle<float2, false, 1, 1>	yy;
		Swizzle<float2, true, 1, 2>		yz;
		Swizzle<float2, true, 2, 0>		zx;
		Swizzle<float2, true, 2, 1>		zy;
		Swizzle<float2, false, 2, 2>	zz;

		Swizzle<float3, false, 0, 0, 0>	 xxx;
		Swizzle<float3, false, 0, 0, 1>	 xxy;
		Swizzle<float3, false, 0, 0, 2>	 xxz;
		Swizzle<float3, false, 0, 1, 0>	 xyx;
		Swizzle<float3, false, 0, 1, 1>	 xyy;
		Swizzle<float3, true, 0, 1, 2>	 xyz;
		Swizzle<float3, false, 0, 2, 0>	 xzx;
		Swizzle<float3, true, 0, 2, 1>	 xzy;
		Swizzle<float3, false, 0, 2, 2>	 xzz;
		Swizzle<float3, false, 1, 0, 0>	 yxx;
		Swizzle<float3, false, 1, 0, 1>	 yxy;
		Swizzle<float3, true, 1, 0, 2>	 yxz;
		Swizzle<float3, false, 1, 1, 0>	 yyx;
		Swizzle<float3, false, 1, 1, 1>	 yyy;
		Swizzle<float3, false, 1, 1, 2>	 yyz;
		Swizzle<float3, true, 1, 2, 0>	 yzx;
		Swizzle<float3, false, 1, 2, 1>	 yzy;
		Swizzle<float3, false, 1, 2, 2>	 yzz;
		Swizzle<float3, false, 2, 0, 0>	 zxx;
		Swizzle<float3, true, 2, 0, 1>	 zxy;
		Swizzle<float3, false, 2, 0, 2>	 zxz;
		Swizzle<float3, true, 2, 1, 0>	 zyx;
		Swizzle<float3, false, 2, 1, 1>	 zyy;
		Swizzle<float3, false, 2, 1, 2>	 zyz;
		Swizzle<float3, false, 2, 2, 0>	 zzx;
		Swizzle<float3, false, 2, 2, 1>	 zzy;
		Swizzle<float3, false, 2, 2, 2>	 zzz;
	};
};

}
