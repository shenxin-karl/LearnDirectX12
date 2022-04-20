#pragma once
#include <ostream>
#include <DirectXMath.h>
#include "Swizzle.hpp"
#include "VectorHelper.h"
#include "Scalar.hpp"

namespace Math {

namespace DX = DirectX;

class alignas(sizeof(DX::XMVECTOR)) BoolVector {
public:
	FORCEINLINE BoolVector(DX::FXMVECTOR vec) noexcept : vec(vec) {
	}
	FORCEINLINE operator DX::XMVECTOR() const {
		return vec;
	}
protected:
	DX::XMVECTOR vec;
};

using Vector2 = float2;

class alignas(sizeof(DX::XMVECTOR)) Vector3 {
public:
	FORCEINLINE Vector3() = default;
	FORCEINLINE Vector3(const Vector3 &) noexcept = default;
	FORCEINLINE Vector3(const float2 &f2, float v) : Vector3(f2.x, f2.y, v) {
	}
	FORCEINLINE explicit Vector3(const float3 &f3) noexcept : Vector3(f3.x, f3.y, f3.z) {
	};
	FORCEINLINE Vector3(DX::XMVECTOR vec) noexcept : vec(vec) {
	}
	FORCEINLINE Vector3(const Scalar &s) noexcept : vec(s) {
	}
	FORCEINLINE Vector3(float x, float y, float z) noexcept {
		vec = DX::XMVectorSet(x, y, z, 0.0);
	}
	FORCEINLINE explicit Vector3(float val) noexcept {
		vec = DX::XMVectorSet(val, val, val, 0.0);
	}
	FORCEINLINE Vector3 &operator=(const Vector3 &other) noexcept {
		vec = other.vec;
		return *this;
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
	FORCEINLINE operator DX::XMVECTOR() const noexcept {
		return vec;
	}
	FORCEINLINE explicit operator float3() const noexcept {
		return xyz;
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Vector3 &v) noexcept {
		os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os;
	}
public:
	union {
		DX::XMVECTOR vec;
		#define VEC3
			#include "VectorMember.ini"
		#undef VEC3
	};
};

class alignas(sizeof(DX::XMVECTOR)) Vector4 {
public:
	FORCEINLINE Vector4() = default;
	FORCEINLINE Vector4(const Vector4 &) noexcept = default;
	FORCEINLINE Vector4(const float3 &f3, float v) noexcept : Vector4(f3.x, f3.y, f3.z, v) {
	};
	FORCEINLINE explicit Vector4(const float4 &f4) noexcept : Vector4(f4.x, f4.y, f4.z, f4.w) {
	};
	FORCEINLINE Vector4(DX::XMVECTOR vec) noexcept : vec(vec) {
	}
	FORCEINLINE Vector4(const Scalar &s) noexcept {
		vec = DX::XMVECTOR(s);
	}
	FORCEINLINE Vector4(float x, float y, float z, float w) noexcept {
		vec = DX::XMVectorSet(x, y, z, w);
	}
	FORCEINLINE explicit Vector4(float val) noexcept {
		vec = DX::XMVectorSet(val, val, val, val);
	}
	FORCEINLINE Vector4 &operator=(const Vector4 & other) noexcept {
		vec = other.vec;
		return *this;
	}
	FORCEINLINE float &operator[](size_t index) noexcept {
		assert(index < 4);
		return vec.m128_f32[index];
	}
	FORCEINLINE float operator[](size_t index) const noexcept {
		assert(index < 4);
		return vec.m128_f32[index];
	}
	FORCEINLINE DX::XMVECTOR *operator&() noexcept {
		return &vec;
	}
	FORCEINLINE const DX::XMVECTOR *operator&() const noexcept {
		return &vec;
	}
	FORCEINLINE operator DX::XMVECTOR() const noexcept {
		return vec;
	}
	FORCEINLINE explicit operator float3() const noexcept {
		return xyz;
	}
	FORCEINLINE explicit operator float4() const noexcept {
		return xyzw;
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Vector4 &v) noexcept {
		os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
		return os;
	}
public:
	union {
		DX::XMVECTOR vec;
		#define VEC4
			#include "VectorMember.ini"
		#undef VEC4
	};
};

#include "VectorFunton.ini"

}
