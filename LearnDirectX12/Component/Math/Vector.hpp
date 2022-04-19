#include <ostream>
#include <DirectXMath.h>
#include "Swizzle.hpp"
#include "VectorHelper.h"
#include "Scalar.hpp"

namespace Math {

namespace DX = DirectX;

class alignas(sizeof(DX::XMVECTOR)) BoolVector {
public:
	FORCEINLINE BoolVector(FXMVECTOR vec) noexcept : vec(vec) {
	}
	FORCEINLINE operator XMVECTOR() const {
		return vec;
	}
protected:
	XMVECTOR vec;
};

using Vector2 = float2;

class alignas(sizeof(DX::XMVECTOR)) Vector3 {
public:
	FORCEINLINE Vector3() = default;
	FORCEINLINE Vector3(const Vector3 &) noexcept = default;
	FORCEINLINE Vector3(const float2 &f2, float v) : Vector3(f2.x, f2.y, v) {
	}
	FORCEINLINE Vector3(const float3 &f3) noexcept : Vector3(f3.x, f3.y, f3.z) {
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

class alignas(sizeof(DX::XMVECTOR)) Vector4 {
public:
	FORCEINLINE Vector4() = default;
	FORCEINLINE Vector4(const Vector4 &) noexcept = default;
	FORCEINLINE Vector4(const float3 &f3, float v) noexcept : Vector4(f3.x, f3.y, f3.z, v) {
	};
	FORCEINLINE Vector4(const float4 &f4) noexcept : Vector4(f4.x, f4.y, f4.z, f4.w) {
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
		Swizzle<float, true, 0>		x;
		Swizzle<float, true, 1>		y;
		Swizzle<float, true, 2>		z;
		Swizzle<float, true, 4>		w;

		Swizzle<float2, false, 0, 0>	xx;
		Swizzle<float2, true, 0, 1>		xy;
		Swizzle<float2, true, 0, 2>		xz;
		Swizzle<float2, true, 0, 3>		xw;
		Swizzle<float2, true, 1, 0>		yx;
		Swizzle<float2, false, 1, 1>	yy;
		Swizzle<float2, true, 1, 2>		yz;
		Swizzle<float2, true, 1, 3>		yw;
		Swizzle<float2, true, 2, 0>		zx;
		Swizzle<float2, true, 2, 1>		zy;
		Swizzle<float2, false, 2, 2>	zz;
		Swizzle<float2, true, 2, 3>		zw;
		Swizzle<float2, true, 3, 0>		wx;
		Swizzle<float2, true, 3, 1>		wy;
		Swizzle<float2, true, 3, 2>		wz;
		Swizzle<float2, false, 3, 3>	ww;

		Swizzle<float3, false, 0, 0, 0>	 xxx;
		Swizzle<float3, false, 0, 0, 1>	 xxy;
		Swizzle<float3, false, 0, 0, 2>	 xxz;
		Swizzle<float3, false, 0, 0, 3>	 xxw;
		Swizzle<float3, false, 0, 1, 0>	 xyx;
		Swizzle<float3, false, 0, 1, 1>	 xyy;
		Swizzle<float3, true, 0, 1, 2>	 xyz;
		Swizzle<float3, true, 0, 1, 3>	 xyw;
		Swizzle<float3, false, 0, 2, 0>	 xzx;
		Swizzle<float3, true, 0, 2, 1>	 xzy;
		Swizzle<float3, false, 0, 2, 2>	 xzz;
		Swizzle<float3, true, 0, 2, 3>	 xzw;
		Swizzle<float3, false, 0, 3, 0>	 xwx;
		Swizzle<float3, true, 0, 3, 1>	 xwy;
		Swizzle<float3, true, 0, 3, 2>	 xwz;
		Swizzle<float3, false, 0, 3, 3>	 xww;
		Swizzle<float3, false, 1, 0, 0>	 yxx;
		Swizzle<float3, false, 1, 0, 1>	 yxy;
		Swizzle<float3, true, 1, 0, 2>	 yxz;
		Swizzle<float3, true, 1, 0, 3>	 yxw;
		Swizzle<float3, false, 1, 1, 0>	 yyx;
		Swizzle<float3, false, 1, 1, 1>	 yyy;
		Swizzle<float3, false, 1, 1, 2>	 yyz;
		Swizzle<float3, false, 1, 1, 3>	 yyw;
		Swizzle<float3, true, 1, 2, 0>	 yzx;
		Swizzle<float3, false, 1, 2, 1>	 yzy;
		Swizzle<float3, false, 1, 2, 2>	 yzz;
		Swizzle<float3, true, 1, 2, 3>	 yzw;
		Swizzle<float3, true, 1, 3, 0>	 ywx;
		Swizzle<float3, false, 1, 3, 1>	 ywy;
		Swizzle<float3, true, 1, 3, 2>	 ywz;
		Swizzle<float3, false, 1, 3, 3>	 yww;
		Swizzle<float3, false, 2, 0, 0>	 zxx;
		Swizzle<float3, true, 2, 0, 1>	 zxy;
		Swizzle<float3, false, 2, 0, 2>	 zxz;
		Swizzle<float3, true, 2, 0, 3>	 zxw;
		Swizzle<float3, true, 2, 1, 0>	 zyx;
		Swizzle<float3, false, 2, 1, 1>	 zyy;
		Swizzle<float3, false, 2, 1, 2>	 zyz;
		Swizzle<float3, true, 2, 1, 3>	 zyw;
		Swizzle<float3, false, 2, 2, 0>	 zzx;
		Swizzle<float3, false, 2, 2, 1>	 zzy;
		Swizzle<float3, false, 2, 2, 2>	 zzz;
		Swizzle<float3, false, 2, 2, 3>	 zzw;
		Swizzle<float3, true, 2, 3, 0>	 zwx;
		Swizzle<float3, true, 2, 3, 1>	 zwy;
		Swizzle<float3, false, 2, 3, 2>	 zwz;
		Swizzle<float3, false, 2, 3, 3>	 zww;
		Swizzle<float3, false, 3, 0, 0>	 wxx;
		Swizzle<float3, true, 3, 0, 1>	 wxy;
		Swizzle<float3, true, 3, 0, 2>	 wxz;
		Swizzle<float3, false, 3, 0, 3>	 wxw;
		Swizzle<float3, true, 3, 1, 0>	 wyx;
		Swizzle<float3, false, 3, 1, 1>	 wyy;
		Swizzle<float3, true, 3, 1, 2>	 wyz;
		Swizzle<float3, false, 3, 1, 3>	 wyw;
		Swizzle<float3, true, 3, 2, 0>	 wzx;
		Swizzle<float3, true, 3, 2, 1>	 wzy;
		Swizzle<float3, false, 3, 2, 2>	 wzz;
		Swizzle<float3, false, 3, 2, 3>	 wzw;
		Swizzle<float3, false, 3, 3, 0>	 wwx;
		Swizzle<float3, false, 3, 3, 1>	 wwy;
		Swizzle<float3, false, 3, 3, 2>	 wwz;
		Swizzle<float3, false, 3, 3, 3>	 www;

		Swizzle<float4, false, 0, 0, 0, 0>	 xxxx;
		Swizzle<float4, false, 0, 0, 0, 1>	 xxxy;
		Swizzle<float4, false, 0, 0, 0, 2>	 xxxz;
		Swizzle<float4, false, 0, 0, 0, 3>	 xxxw;
		Swizzle<float4, false, 0, 0, 1, 0>	 xxyx;
		Swizzle<float4, false, 0, 0, 1, 1>	 xxyy;
		Swizzle<float4, false, 0, 0, 1, 2>	 xxyz;
		Swizzle<float4, false, 0, 0, 1, 3>	 xxyw;
		Swizzle<float4, false, 0, 0, 2, 0>	 xxzx;
		Swizzle<float4, false, 0, 0, 2, 1>	 xxzy;
		Swizzle<float4, false, 0, 0, 2, 2>	 xxzz;
		Swizzle<float4, false, 0, 0, 2, 3>	 xxzw;
		Swizzle<float4, false, 0, 0, 3, 0>	 xxwx;
		Swizzle<float4, false, 0, 0, 3, 1>	 xxwy;
		Swizzle<float4, false, 0, 0, 3, 2>	 xxwz;
		Swizzle<float4, false, 0, 0, 3, 3>	 xxww;
		Swizzle<float4, false, 0, 1, 0, 0>	 xyxx;
		Swizzle<float4, false, 0, 1, 0, 1>	 xyxy;
		Swizzle<float4, false, 0, 1, 0, 2>	 xyxz;
		Swizzle<float4, false, 0, 1, 0, 3>	 xyxw;
		Swizzle<float4, false, 0, 1, 1, 0>	 xyyx;
		Swizzle<float4, false, 0, 1, 1, 1>	 xyyy;
		Swizzle<float4, false, 0, 1, 1, 2>	 xyyz;
		Swizzle<float4, false, 0, 1, 1, 3>	 xyyw;
		Swizzle<float4, false, 0, 1, 2, 0>	 xyzx;
		Swizzle<float4, false, 0, 1, 2, 1>	 xyzy;
		Swizzle<float4, false, 0, 1, 2, 2>	 xyzz;
		Swizzle<float4, true, 0, 1, 2, 3>	 xyzw;
		Swizzle<float4, false, 0, 1, 3, 0>	 xywx;
		Swizzle<float4, false, 0, 1, 3, 1>	 xywy;
		Swizzle<float4, true, 0, 1, 3, 2>	 xywz;
		Swizzle<float4, false, 0, 1, 3, 3>	 xyww;
		Swizzle<float4, false, 0, 2, 0, 0>	 xzxx;
		Swizzle<float4, false, 0, 2, 0, 1>	 xzxy;
		Swizzle<float4, false, 0, 2, 0, 2>	 xzxz;
		Swizzle<float4, false, 0, 2, 0, 3>	 xzxw;
		Swizzle<float4, false, 0, 2, 1, 0>	 xzyx;
		Swizzle<float4, false, 0, 2, 1, 1>	 xzyy;
		Swizzle<float4, false, 0, 2, 1, 2>	 xzyz;
		Swizzle<float4, true, 0, 2, 1, 3>	 xzyw;
		Swizzle<float4, false, 0, 2, 2, 0>	 xzzx;
		Swizzle<float4, false, 0, 2, 2, 1>	 xzzy;
		Swizzle<float4, false, 0, 2, 2, 2>	 xzzz;
		Swizzle<float4, false, 0, 2, 2, 3>	 xzzw;
		Swizzle<float4, false, 0, 2, 3, 0>	 xzwx;
		Swizzle<float4, true, 0, 2, 3, 1>	 xzwy;
		Swizzle<float4, false, 0, 2, 3, 2>	 xzwz;
		Swizzle<float4, false, 0, 2, 3, 3>	 xzww;
		Swizzle<float4, false, 0, 3, 0, 0>	 xwxx;
		Swizzle<float4, false, 0, 3, 0, 1>	 xwxy;
		Swizzle<float4, false, 0, 3, 0, 2>	 xwxz;
		Swizzle<float4, false, 0, 3, 0, 3>	 xwxw;
		Swizzle<float4, false, 0, 3, 1, 0>	 xwyx;
		Swizzle<float4, false, 0, 3, 1, 1>	 xwyy;
		Swizzle<float4, true, 0, 3, 1, 2>	 xwyz;
		Swizzle<float4, false, 0, 3, 1, 3>	 xwyw;
		Swizzle<float4, false, 0, 3, 2, 0>	 xwzx;
		Swizzle<float4, true, 0, 3, 2, 1>	 xwzy;
		Swizzle<float4, false, 0, 3, 2, 2>	 xwzz;
		Swizzle<float4, false, 0, 3, 2, 3>	 xwzw;
		Swizzle<float4, false, 0, 3, 3, 0>	 xwwx;
		Swizzle<float4, false, 0, 3, 3, 1>	 xwwy;
		Swizzle<float4, false, 0, 3, 3, 2>	 xwwz;
		Swizzle<float4, false, 0, 3, 3, 3>	 xwww;
		Swizzle<float4, false, 1, 0, 0, 0>	 yxxx;
		Swizzle<float4, false, 1, 0, 0, 1>	 yxxy;
		Swizzle<float4, false, 1, 0, 0, 2>	 yxxz;
		Swizzle<float4, false, 1, 0, 0, 3>	 yxxw;
		Swizzle<float4, false, 1, 0, 1, 0>	 yxyx;
		Swizzle<float4, false, 1, 0, 1, 1>	 yxyy;
		Swizzle<float4, false, 1, 0, 1, 2>	 yxyz;
		Swizzle<float4, false, 1, 0, 1, 3>	 yxyw;
		Swizzle<float4, false, 1, 0, 2, 0>	 yxzx;
		Swizzle<float4, false, 1, 0, 2, 1>	 yxzy;
		Swizzle<float4, false, 1, 0, 2, 2>	 yxzz;
		Swizzle<float4, true, 1, 0, 2, 3>	 yxzw;
		Swizzle<float4, false, 1, 0, 3, 0>	 yxwx;
		Swizzle<float4, false, 1, 0, 3, 1>	 yxwy;
		Swizzle<float4, true, 1, 0, 3, 2>	 yxwz;
		Swizzle<float4, false, 1, 0, 3, 3>	 yxww;
		Swizzle<float4, false, 1, 1, 0, 0>	 yyxx;
		Swizzle<float4, false, 1, 1, 0, 1>	 yyxy;
		Swizzle<float4, false, 1, 1, 0, 2>	 yyxz;
		Swizzle<float4, false, 1, 1, 0, 3>	 yyxw;
		Swizzle<float4, false, 1, 1, 1, 0>	 yyyx;
		Swizzle<float4, false, 1, 1, 1, 1>	 yyyy;
		Swizzle<float4, false, 1, 1, 1, 2>	 yyyz;
		Swizzle<float4, false, 1, 1, 1, 3>	 yyyw;
		Swizzle<float4, false, 1, 1, 2, 0>	 yyzx;
		Swizzle<float4, false, 1, 1, 2, 1>	 yyzy;
		Swizzle<float4, false, 1, 1, 2, 2>	 yyzz;
		Swizzle<float4, false, 1, 1, 2, 3>	 yyzw;
		Swizzle<float4, false, 1, 1, 3, 0>	 yywx;
		Swizzle<float4, false, 1, 1, 3, 1>	 yywy;
		Swizzle<float4, false, 1, 1, 3, 2>	 yywz;
		Swizzle<float4, false, 1, 1, 3, 3>	 yyww;
		Swizzle<float4, false, 1, 2, 0, 0>	 yzxx;
		Swizzle<float4, false, 1, 2, 0, 1>	 yzxy;
		Swizzle<float4, false, 1, 2, 0, 2>	 yzxz;
		Swizzle<float4, true, 1, 2, 0, 3>	 yzxw;
		Swizzle<float4, false, 1, 2, 1, 0>	 yzyx;
		Swizzle<float4, false, 1, 2, 1, 1>	 yzyy;
		Swizzle<float4, false, 1, 2, 1, 2>	 yzyz;
		Swizzle<float4, false, 1, 2, 1, 3>	 yzyw;
		Swizzle<float4, false, 1, 2, 2, 0>	 yzzx;
		Swizzle<float4, false, 1, 2, 2, 1>	 yzzy;
		Swizzle<float4, false, 1, 2, 2, 2>	 yzzz;
		Swizzle<float4, false, 1, 2, 2, 3>	 yzzw;
		Swizzle<float4, true, 1, 2, 3, 0>	 yzwx;
		Swizzle<float4, false, 1, 2, 3, 1>	 yzwy;
		Swizzle<float4, false, 1, 2, 3, 2>	 yzwz;
		Swizzle<float4, false, 1, 2, 3, 3>	 yzww;
		Swizzle<float4, false, 1, 3, 0, 0>	 ywxx;
		Swizzle<float4, false, 1, 3, 0, 1>	 ywxy;
		Swizzle<float4, true, 1, 3, 0, 2>	 ywxz;
		Swizzle<float4, false, 1, 3, 0, 3>	 ywxw;
		Swizzle<float4, false, 1, 3, 1, 0>	 ywyx;
		Swizzle<float4, false, 1, 3, 1, 1>	 ywyy;
		Swizzle<float4, false, 1, 3, 1, 2>	 ywyz;
		Swizzle<float4, false, 1, 3, 1, 3>	 ywyw;
		Swizzle<float4, true, 1, 3, 2, 0>	 ywzx;
		Swizzle<float4, false, 1, 3, 2, 1>	 ywzy;
		Swizzle<float4, false, 1, 3, 2, 2>	 ywzz;
		Swizzle<float4, false, 1, 3, 2, 3>	 ywzw;
		Swizzle<float4, false, 1, 3, 3, 0>	 ywwx;
		Swizzle<float4, false, 1, 3, 3, 1>	 ywwy;
		Swizzle<float4, false, 1, 3, 3, 2>	 ywwz;
		Swizzle<float4, false, 1, 3, 3, 3>	 ywww;
		Swizzle<float4, false, 2, 0, 0, 0>	 zxxx;
		Swizzle<float4, false, 2, 0, 0, 1>	 zxxy;
		Swizzle<float4, false, 2, 0, 0, 2>	 zxxz;
		Swizzle<float4, false, 2, 0, 0, 3>	 zxxw;
		Swizzle<float4, false, 2, 0, 1, 0>	 zxyx;
		Swizzle<float4, false, 2, 0, 1, 1>	 zxyy;
		Swizzle<float4, false, 2, 0, 1, 2>	 zxyz;
		Swizzle<float4, true, 2, 0, 1, 3>	 zxyw;
		Swizzle<float4, false, 2, 0, 2, 0>	 zxzx;
		Swizzle<float4, false, 2, 0, 2, 1>	 zxzy;
		Swizzle<float4, false, 2, 0, 2, 2>	 zxzz;
		Swizzle<float4, false, 2, 0, 2, 3>	 zxzw;
		Swizzle<float4, false, 2, 0, 3, 0>	 zxwx;
		Swizzle<float4, true, 2, 0, 3, 1>	 zxwy;
		Swizzle<float4, false, 2, 0, 3, 2>	 zxwz;
		Swizzle<float4, false, 2, 0, 3, 3>	 zxww;
		Swizzle<float4, false, 2, 1, 0, 0>	 zyxx;
		Swizzle<float4, false, 2, 1, 0, 1>	 zyxy;
		Swizzle<float4, false, 2, 1, 0, 2>	 zyxz;
		Swizzle<float4, true, 2, 1, 0, 3>	 zyxw;
		Swizzle<float4, false, 2, 1, 1, 0>	 zyyx;
		Swizzle<float4, false, 2, 1, 1, 1>	 zyyy;
		Swizzle<float4, false, 2, 1, 1, 2>	 zyyz;
		Swizzle<float4, false, 2, 1, 1, 3>	 zyyw;
		Swizzle<float4, false, 2, 1, 2, 0>	 zyzx;
		Swizzle<float4, false, 2, 1, 2, 1>	 zyzy;
		Swizzle<float4, false, 2, 1, 2, 2>	 zyzz;
		Swizzle<float4, false, 2, 1, 2, 3>	 zyzw;
		Swizzle<float4, true, 2, 1, 3, 0>	 zywx;
		Swizzle<float4, false, 2, 1, 3, 1>	 zywy;
		Swizzle<float4, false, 2, 1, 3, 2>	 zywz;
		Swizzle<float4, false, 2, 1, 3, 3>	 zyww;
		Swizzle<float4, false, 2, 2, 0, 0>	 zzxx;
		Swizzle<float4, false, 2, 2, 0, 1>	 zzxy;
		Swizzle<float4, false, 2, 2, 0, 2>	 zzxz;
		Swizzle<float4, false, 2, 2, 0, 3>	 zzxw;
		Swizzle<float4, false, 2, 2, 1, 0>	 zzyx;
		Swizzle<float4, false, 2, 2, 1, 1>	 zzyy;
		Swizzle<float4, false, 2, 2, 1, 2>	 zzyz;
		Swizzle<float4, false, 2, 2, 1, 3>	 zzyw;
		Swizzle<float4, false, 2, 2, 2, 0>	 zzzx;
		Swizzle<float4, false, 2, 2, 2, 1>	 zzzy;
		Swizzle<float4, false, 2, 2, 2, 2>	 zzzz;
		Swizzle<float4, false, 2, 2, 2, 3>	 zzzw;
		Swizzle<float4, false, 2, 2, 3, 0>	 zzwx;
		Swizzle<float4, false, 2, 2, 3, 1>	 zzwy;
		Swizzle<float4, false, 2, 2, 3, 2>	 zzwz;
		Swizzle<float4, false, 2, 2, 3, 3>	 zzww;
		Swizzle<float4, false, 2, 3, 0, 0>	 zwxx;
		Swizzle<float4, true, 2, 3, 0, 1>	 zwxy;
		Swizzle<float4, false, 2, 3, 0, 2>	 zwxz;
		Swizzle<float4, false, 2, 3, 0, 3>	 zwxw;
		Swizzle<float4, true, 2, 3, 1, 0>	 zwyx;
		Swizzle<float4, false, 2, 3, 1, 1>	 zwyy;
		Swizzle<float4, false, 2, 3, 1, 2>	 zwyz;
		Swizzle<float4, false, 2, 3, 1, 3>	 zwyw;
		Swizzle<float4, false, 2, 3, 2, 0>	 zwzx;
		Swizzle<float4, false, 2, 3, 2, 1>	 zwzy;
		Swizzle<float4, false, 2, 3, 2, 2>	 zwzz;
		Swizzle<float4, false, 2, 3, 2, 3>	 zwzw;
		Swizzle<float4, false, 2, 3, 3, 0>	 zwwx;
		Swizzle<float4, false, 2, 3, 3, 1>	 zwwy;
		Swizzle<float4, false, 2, 3, 3, 2>	 zwwz;
		Swizzle<float4, false, 2, 3, 3, 3>	 zwww;
		Swizzle<float4, false, 3, 0, 0, 0>	 wxxx;
		Swizzle<float4, false, 3, 0, 0, 1>	 wxxy;
		Swizzle<float4, false, 3, 0, 0, 2>	 wxxz;
		Swizzle<float4, false, 3, 0, 0, 3>	 wxxw;
		Swizzle<float4, false, 3, 0, 1, 0>	 wxyx;
		Swizzle<float4, false, 3, 0, 1, 1>	 wxyy;
		Swizzle<float4, true, 3, 0, 1, 2>	 wxyz;
		Swizzle<float4, false, 3, 0, 1, 3>	 wxyw;
		Swizzle<float4, false, 3, 0, 2, 0>	 wxzx;
		Swizzle<float4, true, 3, 0, 2, 1>	 wxzy;
		Swizzle<float4, false, 3, 0, 2, 2>	 wxzz;
		Swizzle<float4, false, 3, 0, 2, 3>	 wxzw;
		Swizzle<float4, false, 3, 0, 3, 0>	 wxwx;
		Swizzle<float4, false, 3, 0, 3, 1>	 wxwy;
		Swizzle<float4, false, 3, 0, 3, 2>	 wxwz;
		Swizzle<float4, false, 3, 0, 3, 3>	 wxww;
		Swizzle<float4, false, 3, 1, 0, 0>	 wyxx;
		Swizzle<float4, false, 3, 1, 0, 1>	 wyxy;
		Swizzle<float4, true, 3, 1, 0, 2>	 wyxz;
		Swizzle<float4, false, 3, 1, 0, 3>	 wyxw;
		Swizzle<float4, false, 3, 1, 1, 0>	 wyyx;
		Swizzle<float4, false, 3, 1, 1, 1>	 wyyy;
		Swizzle<float4, false, 3, 1, 1, 2>	 wyyz;
		Swizzle<float4, false, 3, 1, 1, 3>	 wyyw;
		Swizzle<float4, true, 3, 1, 2, 0>	 wyzx;
		Swizzle<float4, false, 3, 1, 2, 1>	 wyzy;
		Swizzle<float4, false, 3, 1, 2, 2>	 wyzz;
		Swizzle<float4, false, 3, 1, 2, 3>	 wyzw;
		Swizzle<float4, false, 3, 1, 3, 0>	 wywx;
		Swizzle<float4, false, 3, 1, 3, 1>	 wywy;
		Swizzle<float4, false, 3, 1, 3, 2>	 wywz;
		Swizzle<float4, false, 3, 1, 3, 3>	 wyww;
		Swizzle<float4, false, 3, 2, 0, 0>	 wzxx;
		Swizzle<float4, true, 3, 2, 0, 1>	 wzxy;
		Swizzle<float4, false, 3, 2, 0, 2>	 wzxz;
		Swizzle<float4, false, 3, 2, 0, 3>	 wzxw;
		Swizzle<float4, true, 3, 2, 1, 0>	 wzyx;
		Swizzle<float4, false, 3, 2, 1, 1>	 wzyy;
		Swizzle<float4, false, 3, 2, 1, 2>	 wzyz;
		Swizzle<float4, false, 3, 2, 1, 3>	 wzyw;
		Swizzle<float4, false, 3, 2, 2, 0>	 wzzx;
		Swizzle<float4, false, 3, 2, 2, 1>	 wzzy;
		Swizzle<float4, false, 3, 2, 2, 2>	 wzzz;
		Swizzle<float4, false, 3, 2, 2, 3>	 wzzw;
		Swizzle<float4, false, 3, 2, 3, 0>	 wzwx;
		Swizzle<float4, false, 3, 2, 3, 1>	 wzwy;
		Swizzle<float4, false, 3, 2, 3, 2>	 wzwz;
		Swizzle<float4, false, 3, 2, 3, 3>	 wzww;
		Swizzle<float4, false, 3, 3, 0, 0>	 wwxx;
		Swizzle<float4, false, 3, 3, 0, 1>	 wwxy;
		Swizzle<float4, false, 3, 3, 0, 2>	 wwxz;
		Swizzle<float4, false, 3, 3, 0, 3>	 wwxw;
		Swizzle<float4, false, 3, 3, 1, 0>	 wwyx;
		Swizzle<float4, false, 3, 3, 1, 1>	 wwyy;
		Swizzle<float4, false, 3, 3, 1, 2>	 wwyz;
		Swizzle<float4, false, 3, 3, 1, 3>	 wwyw;
		Swizzle<float4, false, 3, 3, 2, 0>	 wwzx;
		Swizzle<float4, false, 3, 3, 2, 1>	 wwzy;
		Swizzle<float4, false, 3, 3, 2, 2>	 wwzz;
		Swizzle<float4, false, 3, 3, 2, 3>	 wwzw;
		Swizzle<float4, false, 3, 3, 3, 0>	 wwwx;
		Swizzle<float4, false, 3, 3, 3, 1>	 wwwy;
		Swizzle<float4, false, 3, 3, 3, 2>	 wwwz;
		Swizzle<float4, false, 3, 3, 3, 3>	 wwww;
	};
};


#define DECLARE_OPERATION(TYPE) \
FORCEINLINE TYPE operator+(const TYPE &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorAdd(lhs, rhs);\
}\
FORCEINLINE TYPE operator-(const TYPE &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorSubtract(lhs, rhs);\
}\
FORCEINLINE TYPE operator*(const TYPE &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorMultiply(lhs, rhs);\
}\
FORCEINLINE TYPE operator/(const TYPE &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorDivide(lhs, rhs);\
}\
FORCEINLINE TYPE operator+(const TYPE &lhs, const Scalar &rhs) noexcept {\
	return DX::XMVectorAdd(lhs, DX::XMVECTOR(rhs)); \
}\
FORCEINLINE TYPE operator-(const TYPE &lhs, const Scalar &rhs) noexcept {\
	return DX::XMVectorSubtract(lhs, DX::XMVECTOR(rhs)); \
}\
FORCEINLINE TYPE operator*(const TYPE &lhs, const Scalar &rhs) noexcept {\
	return DX::XMVectorMultiply(lhs, DX::XMVECTOR(rhs)); \
}\
FORCEINLINE TYPE operator/(const TYPE &lhs, const Scalar &rhs) noexcept {\
	return DX::XMVectorDivide(lhs, DX::XMVECTOR(rhs)); \
}\
FORCEINLINE TYPE operator+(const Scalar &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorAdd(DX::XMVECTOR(lhs), rhs);\
}\
FORCEINLINE TYPE operator-(const Scalar &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorSubtract(DX::XMVECTOR(lhs), rhs);\
}\
FORCEINLINE TYPE operator*(const Scalar &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorMultiply(DX::XMVECTOR(lhs), rhs);\
}\
FORCEINLINE TYPE operator/(const Scalar &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorDivide(DX::XMVECTOR(lhs), rhs);\
}\
FORCEINLINE TYPE operator+(const float &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorAdd(TYPE(lhs), rhs);\
}\
FORCEINLINE TYPE operator-(const float &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorSubtract(TYPE(lhs), rhs);\
}\
FORCEINLINE TYPE operator*(const float &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorMultiply(TYPE(lhs), rhs);\
}\
FORCEINLINE TYPE operator/(const float &lhs, const TYPE &rhs) noexcept {\
	return DX::XMVectorDivide(TYPE(lhs), rhs);\
}\
FORCEINLINE TYPE operator+(const TYPE &lhs, const float &rhs) noexcept {\
	return DX::XMVectorAdd(lhs, TYPE(rhs));\
}\
FORCEINLINE TYPE operator-(const TYPE &lhs, const float &rhs) noexcept {\
	return DX::XMVectorSubtract(lhs, TYPE(rhs));\
}\
FORCEINLINE TYPE operator*(const TYPE &lhs, const float &rhs) noexcept {\
	return DX::XMVectorMultiply(lhs, TYPE(rhs));\
}\
FORCEINLINE TYPE operator/(const TYPE &lhs, const float &rhs) noexcept {\
	return DX::XMVectorDivide(lhs, TYPE(rhs));\
}\
FORCEINLINE TYPE &operator+=(TYPE &lhs, const TYPE &rhs) noexcept {\
	lhs = lhs + rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator-=(TYPE &lhs, const TYPE &rhs) noexcept {\
	lhs = lhs - rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator*=(TYPE &lhs, const TYPE &rhs) noexcept {\
	lhs = lhs * rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator/=(TYPE &lhs, const TYPE &rhs) noexcept {\
	lhs = lhs / rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator+=(TYPE &lhs, const Scalar &rhs) noexcept {\
	lhs = lhs + rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator-=(TYPE &lhs, const Scalar &rhs) noexcept {\
	lhs = lhs - rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator*=(TYPE &lhs, const Scalar &rhs) noexcept {\
	lhs = lhs * rhs;\
	return lhs;\
}\
FORCEINLINE TYPE &operator/=(TYPE &lhs, const Scalar &rhs) noexcept {\
	lhs = lhs / rhs;\
	return lhs;\
}\
FORCEINLINE TYPE operator-(const TYPE &v) noexcept {\
	return -v;\
}\
FORCEINLINE BoolVector operator<  (TYPE lhs, TYPE rhs) noexcept {\
	return DX::XMVectorLess(lhs, rhs);\
} \
FORCEINLINE BoolVector operator<= (TYPE lhs, TYPE rhs) noexcept {\
	return DX::XMVectorLessOrEqual(lhs, rhs);\
} \
FORCEINLINE BoolVector operator>  (TYPE lhs, TYPE rhs) noexcept {\
	return DX::XMVectorGreater(lhs, rhs);\
} \
FORCEINLINE BoolVector operator>= (TYPE lhs, TYPE rhs) noexcept {\
	return DX::XMVectorGreaterOrEqual(lhs, rhs);\
} \
FORCEINLINE BoolVector operator== (TYPE lhs, TYPE rhs) noexcept {\
	return DX::XMVectorEqual(lhs, rhs);\
} \
FORCEINLINE TYPE sqrt(TYPE s) noexcept {\
	return TYPE(XMVectorSqrt(s));\
} \
FORCEINLINE TYPE inverse(TYPE s) noexcept {\
	return TYPE(XMVectorReciprocal(s));\
} \
FORCEINLINE TYPE inverseSqrt(TYPE s) noexcept {\
	return TYPE(XMVectorReciprocalSqrt(s));\
} \
FORCEINLINE TYPE floor(TYPE s) noexcept {\
	return TYPE(XMVectorFloor(s));\
} \
FORCEINLINE TYPE ceil(TYPE s) noexcept {\
	return TYPE(XMVectorCeiling(s));\
} \
FORCEINLINE TYPE round(TYPE s) noexcept {\
	return TYPE(XMVectorRound(s));\
} \
FORCEINLINE TYPE abs(TYPE s) noexcept {\
	return TYPE(XMVectorAbs(s));\
} \
FORCEINLINE TYPE exp(TYPE s) noexcept {\
	return TYPE(XMVectorExp(s));\
} \
FORCEINLINE TYPE pow(TYPE b, TYPE e) noexcept {\
	return TYPE(XMVectorPow(b, e));\
} \
FORCEINLINE TYPE log(TYPE s) noexcept {\
	return TYPE(XMVectorLog(s));\
} \
FORCEINLINE TYPE sin(TYPE s) noexcept {\
	return TYPE(XMVectorSin(s));\
} \
FORCEINLINE TYPE cos(TYPE s) noexcept {\
	return TYPE(XMVectorCos(s));\
} \
FORCEINLINE TYPE tan(TYPE s) noexcept {\
	return TYPE(XMVectorTan(s));\
} \
FORCEINLINE TYPE asin(TYPE s) noexcept {\
	return TYPE(XMVectorASin(s));\
} \
FORCEINLINE TYPE acos(TYPE s) noexcept {\
	return TYPE(XMVectorACos(s));\
} \
FORCEINLINE TYPE atan(TYPE s) noexcept {\
	return TYPE(XMVectorATan(s));\
} \
FORCEINLINE TYPE atan2(TYPE y, TYPE x) noexcept {\
	return TYPE(XMVectorATan2(y, x));\
} \
FORCEINLINE TYPE lerp(TYPE a, TYPE b, TYPE t) noexcept {\
	return TYPE(XMVectorLerpV(a, b, t));\
} \
FORCEINLINE TYPE lerp(TYPE a, TYPE b, float t) noexcept {\
	return TYPE(XMVectorLerp(a, b, t));\
} \
FORCEINLINE TYPE max(TYPE a, TYPE b) noexcept {\
	return TYPE(XMVectorMax(a, b));\
} \
FORCEINLINE TYPE min(TYPE a, TYPE b) noexcept {\
	return TYPE(XMVectorMin(a, b));\
} \
FORCEINLINE TYPE clamp(TYPE v, TYPE a, TYPE b) noexcept {\
	return min(max(v, a), b);\
}

DECLARE_OPERATION(Vector3)
DECLARE_OPERATION(Vector4)
#undef DECLARE_OPERATION

FORCEINLINE Vector3 normalize(const Vector3 &v) noexcept {
	return DX::XMVector3Normalize(v);
}
FORCEINLINE Scalar dot(Vector3 v1, Vector3 v2) noexcept {
	return Scalar(DX::XMVector3Dot(v1, v2));
}
FORCEINLINE Vector3 cross(Vector3 v0, Vector3 v1) {
	return Scalar(DX::XMVector3Cross(v0, v1));
}
FORCEINLINE Scalar length(Vector3 v) {
	return Scalar(DX::XMVector3Length(v));
}
FORCEINLINE Scalar lengthSquare(Vector3 v) {
	return Scalar(DX::XMVector3LengthSq(v));
}
FORCEINLINE Scalar inverseLength(Vector3 v) {
	return Scalar(DX::XMVector3ReciprocalLength(v));
}

FORCEINLINE Vector4 normalize(const Vector4 &v) noexcept {
	return DX::XMVector4Normalize(v);
}
FORCEINLINE Scalar dot(Vector4 v1, Vector4 v2) noexcept {
	return Scalar(DX::XMVector4Dot(v1, v2));
}
FORCEINLINE Scalar length(Vector4 v) {
	return Scalar(DX::XMVector4Length(v));
}
FORCEINLINE Scalar lengthSquare(Vector4 v) {
	return Scalar(DX::XMVector4LengthSq(v));
}
FORCEINLINE Scalar inverseLength(Vector4 v) {
	return Scalar(DX::XMVector4ReciprocalLength(v));
}

}
