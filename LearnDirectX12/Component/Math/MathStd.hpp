#pragma once
#define NOMINMAX
#include <algorithm>
#include <DirectXMath.h>
#include <ostream>
#undef min
#undef max
#define FORCEINLINE __forceinline

namespace Math {

namespace DX = DirectX;

template<size_t N>
struct FloatStore;

using float2 = FloatStore<2>;
using float3 = FloatStore<3>;
using float4 = FloatStore<4>;
using float3x3 = DX::XMFLOAT3X3;
using float4x3 = DX::XMFLOAT4X3;
using float4x4 = DX::XMFLOAT4X4;

class Vector2;
class Vector3;
class Vector4;
class Matrix3;
class Matrix4;

template<>
struct FloatStore<2> : public DX::XMFLOAT2 {
	using DX::XMFLOAT2::XMFLOAT2;
	using DX::XMFLOAT2::operator=;

	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept;
	FORCEINLINE explicit FloatStore(const Vector3 &vec) noexcept;
	FORCEINLINE explicit FloatStore(const Vector4 &vec) noexcept;
	FORCEINLINE float &operator[](size_t n) noexcept;
	FORCEINLINE float operator[](size_t n) const noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const FloatStore &v) noexcept;
	FORCEINLINE FloatStore operator-() const noexcept;
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept;

	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v) noexcept;

	template<typename T1, typename T2>
		requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float>)
	FORCEINLINE FloatStore(T1 x, T2 y) noexcept;
};

template<>
struct FloatStore<3> : public DX::XMFLOAT3 {
	using DX::XMFLOAT3::XMFLOAT3;
	using DX::XMFLOAT3::operator=;

	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE explicit FloatStore(const Vector3 &vec) noexcept;
	FORCEINLINE explicit FloatStore(const Vector4 &vec) noexcept;
	FORCEINLINE FloatStore(const FloatStore<2> &f2, float z) noexcept;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept;
	FORCEINLINE float &operator[](size_t n) noexcept;
	FORCEINLINE float operator[](size_t n) const noexcept;
	FORCEINLINE explicit FloatStore(const DX::XMVECTORF32 &color) noexcept;
	FORCEINLINE FloatStore operator-() const noexcept;
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept;

	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v);

	template<typename T1, typename T2, typename T3>
		requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
	std::is_convertible_v<T3, float>)
	FORCEINLINE FloatStore(T1 x, T2 y, T3 z) noexcept;

	template<size_t N> requires(N <= 3)
	FORCEINLINE explicit operator FloatStore<N> &() noexcept;

	template<size_t N> requires(N <= 3)
	FORCEINLINE explicit operator const FloatStore<N> &() const noexcept;
};

template<>
struct FloatStore<4> : public DX::XMFLOAT4 {
	using DX::XMFLOAT4::XMFLOAT4;
	using DX::XMFLOAT4::operator=;

	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept;
	FORCEINLINE FloatStore(const FloatStore<2> &f2, float z, float w) noexcept;
	FORCEINLINE FloatStore(const FloatStore<3> &f3, float w) noexcept;
	FORCEINLINE float &operator[](size_t n) noexcept;
	FORCEINLINE float operator[](size_t n) const noexcept;
	FORCEINLINE explicit FloatStore(const DX::XMVECTORF32 &color) noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const FloatStore &v) noexcept;
	FORCEINLINE FloatStore operator-() const noexcept;
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept;

	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v) noexcept;
	template<typename T1, typename T2, typename T3, typename T4>
		requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
	std::is_convertible_v<T3, float> &&std::is_convertible_v<T4, float>)
	FORCEINLINE FloatStore(T1 x, T2 y, T3 z, T4 w) noexcept;

	template<size_t N> requires(N <= 4)
	FORCEINLINE explicit operator FloatStore<N> &() noexcept;

	template<size_t N> requires(N <= 4)
	FORCEINLINE explicit operator const FloatStore<N> &() const noexcept;
};

class alignas(16) Scalar {
public:
	FORCEINLINE Scalar() = default;
	FORCEINLINE Scalar(const Scalar &) = default;
	FORCEINLINE Scalar &operator=(const Scalar &) = default;
	FORCEINLINE Scalar(DX::XMVECTOR vec) noexcept;
	FORCEINLINE explicit Scalar(float v) noexcept;
	FORCEINLINE operator float() const noexcept;
	FORCEINLINE DX::XMVECTOR *operator&() noexcept;
	FORCEINLINE const DX::XMVECTOR *operator&() const noexcept;
	FORCEINLINE operator DX::XMVECTOR() const noexcept;
	FORCEINLINE friend Scalar operator- (Scalar s) noexcept;
	FORCEINLINE friend Scalar operator+ (Scalar s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator- (Scalar s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator* (Scalar s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator/ (Scalar s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator+ (Scalar s1, float s2) noexcept;
	FORCEINLINE friend Scalar operator- (Scalar s1, float s2) noexcept;
	FORCEINLINE friend Scalar operator* (Scalar s1, float s2) noexcept;
	FORCEINLINE friend Scalar operator/ (Scalar s1, float s2) noexcept;
	FORCEINLINE friend Scalar operator+ (float s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator- (float s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator* (float s1, Scalar s2) noexcept;
	FORCEINLINE friend Scalar operator/ (float s1, Scalar s2) noexcept;
private:
	DX::XMVECTOR _vec;
};

template<typename T, bool EnableAssign, size_t...I>
struct Swizzle {
	constexpr static size_t indices[] = { I... };

	Swizzle() noexcept = default;
	template<typename = void> requires(EnableAssign)
	FORCEINLINE T &operator=(const T &other) noexcept;
	FORCEINLINE operator T() const noexcept;
public:
	template<size_t Idx>
	FORCEINLINE float &at() noexcept;

	template<size_t Idx>
	FORCEINLINE float at() const noexcept;
};

class alignas(16) BoolVector {
public:
	FORCEINLINE BoolVector(DX::FXMVECTOR vec) noexcept;
	FORCEINLINE operator DX::XMVECTOR() const;
private:
	DX::XMVECTOR _vec;
};

class Vector2 {
public:
	FORCEINLINE Vector2() noexcept = default;
	FORCEINLINE Vector2(float x, float y) noexcept;
	FORCEINLINE explicit Vector2(float val) noexcept;
	FORCEINLINE explicit Vector2(const float2 &f2) noexcept;
	FORCEINLINE explicit Vector2(const Vector3 &v3) noexcept;
	FORCEINLINE explicit Vector2(const Vector4 &v4) noexcept;
	FORCEINLINE Vector2 &operator=(const Vector2 &other) noexcept;
	FORCEINLINE float &operator[](size_t index) noexcept;
	FORCEINLINE float operator[](size_t index) const noexcept;
	FORCEINLINE explicit operator float2() const noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Vector2 &v) noexcept;
public:
	union {
		struct {
			float x;
			float y;
		};
#define VEC2
#include "VectorMember.ini"
#undef VEC2
	};
};

class alignas(16) Vector3 {
public:
	FORCEINLINE Vector3() = default;
	FORCEINLINE Vector3(const Vector3 &) noexcept = default;
	FORCEINLINE Vector3(const float2 &f2, float v) noexcept;
	FORCEINLINE explicit Vector3(const float3 &f3) noexcept;
	FORCEINLINE Vector3(DX::XMVECTOR vec) noexcept;
	FORCEINLINE Vector3(const Scalar &s) noexcept;
	FORCEINLINE Vector3(float x, float y, float z) noexcept;
	FORCEINLINE explicit Vector3(float val) noexcept;
	FORCEINLINE Vector3 &operator=(const Vector3 &other) noexcept;
	FORCEINLINE float &operator[](size_t index) noexcept;
	FORCEINLINE float operator[](size_t index) const noexcept;
	FORCEINLINE DX::XMVECTOR *operator&() noexcept;
	FORCEINLINE const DX::XMVECTOR *operator&() const noexcept;
	FORCEINLINE operator DX::XMVECTOR() const noexcept;
	FORCEINLINE explicit operator float2() const noexcept;
	FORCEINLINE explicit operator float3() const noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Vector3 &v) noexcept;
public:
	union {
		DX::XMVECTOR _vec;
#define VEC3
#include "VectorMember.ini"
#undef VEC3
	};
};

class alignas(16) Vector4 {
public:
	FORCEINLINE Vector4() = default;
	FORCEINLINE Vector4(const Vector4 &) noexcept = default;
	FORCEINLINE Vector4(const float3 &f3, float v) noexcept;
	FORCEINLINE explicit Vector4(const float4 &f4) noexcept;
	FORCEINLINE Vector4(DX::XMVECTOR vec) noexcept;
	FORCEINLINE Vector4(const Scalar &s) noexcept;
	FORCEINLINE Vector4(float x, float y, float z, float w) noexcept;
	FORCEINLINE explicit Vector4(float val) noexcept;
	FORCEINLINE Vector4 &operator=(const Vector4 &other) noexcept;
	FORCEINLINE float &operator[](size_t index) noexcept;
	FORCEINLINE float operator[](size_t index) const noexcept;
	FORCEINLINE DX::XMVECTOR *operator&() noexcept;
	FORCEINLINE const DX::XMVECTOR *operator&() const noexcept;
	FORCEINLINE operator DX::XMVECTOR() const noexcept;
	FORCEINLINE explicit operator float2() const noexcept;
	FORCEINLINE explicit operator float3() const noexcept;
	FORCEINLINE explicit operator float4() const noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Vector4 &v) noexcept;
public:
	union {
		DX::XMVECTOR vec;
#define VEC4
#include "VectorMember.ini"
#undef VEC4
	};
};

class alignas(16) Matrix3 {
public:
	FORCEINLINE Matrix3() noexcept = default;
	FORCEINLINE Matrix3(const Matrix3 &) noexcept = default;
	FORCEINLINE Matrix3(Matrix3 &&) noexcept = default;
	FORCEINLINE Matrix3 &operator=(const Matrix3 &) = default;
	FORCEINLINE Matrix3 &operator=(Matrix3 &&) = default;
	FORCEINLINE explicit Matrix3(const float3x3 &f3x3) noexcept;
	FORCEINLINE explicit Matrix3(const DX::XMMATRIX &mat) noexcept;
	FORCEINLINE explicit Matrix3(const Matrix4 &mat4) noexcept;
	FORCEINLINE Matrix3(const Vector3 &x, const Vector3 &y, const Vector3 &z) noexcept;
	FORCEINLINE void setX(const Vector3 &x) noexcept;
	FORCEINLINE void setY(const Vector3 &y) noexcept;
	FORCEINLINE void setZ(const Vector3 &z) noexcept;
	FORCEINLINE Vector3 getX() const noexcept;
	FORCEINLINE Vector3 getY() const noexcept;
	FORCEINLINE Vector3 getZ() const noexcept;
	FORCEINLINE explicit operator DX::XMMATRIX() const noexcept;
	FORCEINLINE explicit operator Matrix4() const noexcept;
	FORCEINLINE explicit operator float3x3() const noexcept;
	FORCEINLINE Vector3 &operator[](size_t index) noexcept;
	FORCEINLINE const Vector3 &operator[](size_t index) const noexcept;
	FORCEINLINE Matrix3 operator*(const Scalar &s) const noexcept;
	FORCEINLINE Vector3 operator*(const Vector3 &v) const noexcept;
	FORCEINLINE Matrix3 operator*(const Matrix3 &mat) const noexcept;
	static FORCEINLINE Matrix3 makeZRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix3 makeXRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix3 makeYRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix3 makeZRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix3 makeXRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix3 makeYRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix3 makeAxisRotationByDegree(const Vector3 &axis, float angle) noexcept;
	static FORCEINLINE Matrix3 makeAxisRotationByRadian(const Vector3 &axis, float radian) noexcept;
	static FORCEINLINE Matrix3 makeScale(float scale) noexcept;
	static FORCEINLINE Matrix3 makeScale(float sx, float sy, float sz) noexcept;
	static FORCEINLINE Matrix3 makeScale(const float3 &scale) noexcept;
	static FORCEINLINE Matrix3 makeScale(const Vector3 &scale) noexcept;
	static FORCEINLINE Matrix3 identity() noexcept;
private:
	Vector3 _mat[3];
};
 
class alignas(16) Matrix4 {
public:
	FORCEINLINE Matrix4() noexcept = default;
	FORCEINLINE Matrix4(const Matrix4 &) noexcept = default;
	FORCEINLINE Matrix4(Matrix4 &&) noexcept = default;
	FORCEINLINE Matrix4 &operator=(const Matrix4 &) noexcept = default;
	FORCEINLINE Matrix4 &operator=(Matrix4 &&) noexcept = default;
	FORCEINLINE Matrix4(const DX::XMMATRIX &mat) noexcept;
	FORCEINLINE explicit Matrix4(const float4x4 &f4x4) noexcept;
	FORCEINLINE explicit Matrix4(const Matrix3 &mat3) noexcept;
	FORCEINLINE explicit Matrix4(const float *data) noexcept;
	FORCEINLINE Matrix4(const Vector3 &x, const Vector3 &y, const Vector3 &z, const Vector3 &w) noexcept;
	FORCEINLINE Matrix4(const Vector4&x, const Vector4&y, const Vector4&z, const Vector4&w) noexcept;
	FORCEINLINE const Matrix3 &get3x3() const noexcept;
	FORCEINLINE void set3x3(const Matrix3 &xyz) noexcept;
	FORCEINLINE Vector4 getX() const noexcept;
	FORCEINLINE Vector4 getY() const noexcept;
	FORCEINLINE Vector4 getZ() const noexcept;
	FORCEINLINE Vector4 getW() const noexcept;
	FORCEINLINE void setX(Vector4 x) noexcept;
	FORCEINLINE void setY(Vector4 y) noexcept;
	FORCEINLINE void setZ(Vector4 z) noexcept;
	FORCEINLINE void setW(Vector4 w) noexcept;
	FORCEINLINE explicit operator DX::XMMATRIX() const noexcept;
	FORCEINLINE explicit operator float3x3() const noexcept;
	FORCEINLINE explicit operator float4x3() const noexcept;
	FORCEINLINE explicit operator float4x4() const noexcept;
	FORCEINLINE DX::XMMATRIX *operator&() noexcept;
	FORCEINLINE const DX::XMMATRIX *operator&() const noexcept;
	FORCEINLINE Vector4 operator*(const Vector3 &vec) const noexcept;
	FORCEINLINE Vector4 operator*(const Vector4 &vec) const noexcept;
	FORCEINLINE Matrix4 operator*(const Matrix4 &mat) const noexcept;
	static FORCEINLINE Matrix4 makeZRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix4 makeXRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix4 makeYRotationByRadian(float radian) noexcept;
	static FORCEINLINE Matrix4 makeZRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix4 makeXRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix4 makeYRotationByDegree(float angle) noexcept;
	static FORCEINLINE Matrix4 makeAxisRotationByDegree(const Vector3 &axis, float angle) noexcept;
	static FORCEINLINE Matrix4 makeAxisRotationByRadian(const Vector3 &axis, float radian) noexcept;
	static FORCEINLINE Matrix4 makeScale(float scale) noexcept;
	static FORCEINLINE Matrix4 makeScale(float sx, float sy, float sz) noexcept;
	static FORCEINLINE Matrix4 makeScale(const float3 &scale) noexcept;
	static FORCEINLINE Matrix4 makeScale(const Vector3 &scale) noexcept;
	static FORCEINLINE Matrix4 makeTranslation(const Vector3 &vec) noexcept;
	static FORCEINLINE Matrix4 makeTranslation(float ox, float oy, float oz) noexcept;
	static FORCEINLINE Matrix4 identity() noexcept;
private:
	DX::XMMATRIX _mat;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/// FloatStore<2>
#if 1		
FORCEINLINE FloatStore<2>::FloatStore(DX::FXMVECTOR v) noexcept {
	x = DX::XMVectorGetX(v);
	y = DX::XMVectorGetY(v);
}
FORCEINLINE FloatStore<2>::FloatStore(const Vector3 &vec) noexcept {
	x = vec.x;
	y = vec.y;
}
FORCEINLINE FloatStore<2>::FloatStore(const Vector4 &vec) noexcept {
	x = vec.x;
	y = vec.y;
}
FORCEINLINE float &FloatStore<2>::operator[](size_t n) noexcept {
	assert(n < 2);
	return reinterpret_cast<float *>(this)[n];
}
FORCEINLINE float FloatStore<2>::operator[](size_t n) const noexcept {
	assert(n < 2);
	return reinterpret_cast<const float *>(this)[n];
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const FloatStore<2> &v) noexcept {
	os << '(' << v.x << ", " << v.y << ')';
	return os;
}
FORCEINLINE FloatStore<2> FloatStore<2>::operator-() const noexcept {
	return FloatStore(-x, -y);
}
FORCEINLINE FloatStore<2>::operator DX::XMVECTOR() const noexcept {
	return DX::XMVectorSet(x, y, 0.f, 0.f);
}
template<typename T> requires(std::is_convertible_v<T, float>)
FORCEINLINE FloatStore<2>::FloatStore(T v) noexcept : DX::XMFLOAT2(static_cast<float>(v), static_cast<float>(v)) {
}
template<typename T1, typename T2>
	requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float>)
FORCEINLINE FloatStore<2>::FloatStore(T1 x, T2 y) noexcept : DX::XMFLOAT2(float(x), float(y)) {
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// FloatStore<3>
#if 1	
FORCEINLINE FloatStore<3>::FloatStore(const Vector3 &vec) noexcept {
	*this = vec.operator float3();
}
FORCEINLINE FloatStore<3>::FloatStore(const Vector4 &vec) noexcept {
	*this = vec.operator float3();
}
FORCEINLINE FloatStore<3>::FloatStore(const FloatStore<2> &f2, float z) noexcept : FloatStore(f2.x, f2.y, z) {
}
FORCEINLINE FloatStore<3>::FloatStore(DX::FXMVECTOR v) noexcept {
	x = DX::XMVectorGetX(v);
	y = DX::XMVectorGetY(v);
	z = DX::XMVectorGetZ(v);
}
template<typename T> requires(std::is_convertible_v<T, float>)
FORCEINLINE FloatStore<3>::FloatStore(T v)
	: XMFLOAT3(float(v), float(v), float(v)) {

}
template<typename T1, typename T2, typename T3>
	requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
std::is_convertible_v<T3, float>)
FORCEINLINE FloatStore<3>::FloatStore(T1 x, T2 y, T3 z) noexcept
	: DX::XMFLOAT3(float(x), float(y), float(z)) {

}
template<size_t N> requires(N <= 3)
FORCEINLINE FloatStore<3>::operator FloatStore<N> &() noexcept {
	return reinterpret_cast<FloatStore<N> &>(*this);
}
template<size_t N> requires(N <= 3)
FORCEINLINE FloatStore<3>::operator const FloatStore<N> &() const noexcept {
	return reinterpret_cast<const FloatStore<N> &>(*this);
}
FORCEINLINE float &FloatStore<3>::operator[](size_t n) noexcept {
	assert(n < 3);
	return reinterpret_cast<float *>(this)[n];
}
FORCEINLINE float FloatStore<3>::operator[](size_t n) const noexcept {
	assert(n < 3);
	return reinterpret_cast<const float *>(this)[n];
}
FORCEINLINE FloatStore<3>::FloatStore(const DX::XMVECTORF32 &color) noexcept
: FloatStore(color.operator DirectX::XMVECTOR()) {
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const FloatStore<3> &v) noexcept {
	os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
	return os;
}
FloatStore<3> FloatStore<3>::operator-() const noexcept {
	return FloatStore(-x, -y, -z);
}
FORCEINLINE FloatStore<3>::operator DX::XMVECTOR() const noexcept {
	return DX::XMVectorSet(x, y, z, 0.f);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// FloatStore<4>
#if 1
FORCEINLINE FloatStore<4>::FloatStore(DX::FXMVECTOR v) noexcept {
	x = DX::XMVectorGetX(v);
	y = DX::XMVectorGetY(v);
	z = DX::XMVectorGetZ(v);
	w = DX::XMVectorGetW(v);
}
FORCEINLINE FloatStore<4>::FloatStore(const FloatStore<2> &f2, float z, float w) noexcept : FloatStore(f2.x, f2.y, z, w) {
}
FORCEINLINE FloatStore<4>::FloatStore(const FloatStore<3> &f3, float w) noexcept : DX::XMFLOAT4(f3.x, f3.y, f3.z, w) {
}
FORCEINLINE float &FloatStore<4>::operator[](size_t n) noexcept {
	assert(n < 4);
	return reinterpret_cast<float *>(this)[n];
}
FORCEINLINE float FloatStore<4>::operator[](size_t n) const noexcept {
	assert(n < 4);
	return reinterpret_cast<const float *>(this)[n];
}
FORCEINLINE FloatStore<4>::FloatStore(const DX::XMVECTORF32 &color) noexcept
: FloatStore(color.operator DirectX::XMVECTOR()) {
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const FloatStore<4> &v) noexcept {
	os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
	return os;
}
FORCEINLINE FloatStore<4> FloatStore<4>::operator-() const noexcept {
	return FloatStore(-x, -y, -z, -w);
}
FORCEINLINE FloatStore<4>::operator DX::XMVECTOR() const noexcept {
	return DX::XMLoadFloat4(this);
}
template<typename T> requires(std::is_convertible_v<T, float>)
FORCEINLINE FloatStore<4>::FloatStore(T v) noexcept : DX::XMFLOAT4(float(v), float(v), float(v), float(v)) {
}
template<typename T1, typename T2, typename T3, typename T4>
	requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
std::is_convertible_v<T3, float> &&std::is_convertible_v<T4, float>)
FORCEINLINE FloatStore<4>::FloatStore(T1 x, T2 y, T3 z, T4 w) noexcept : DX::XMFLOAT4(float(x), float(y), float(z), float(w)) {
}
template<size_t N> requires(N <= 4)
FORCEINLINE FloatStore<4>::operator FloatStore<N> &() noexcept {
	return reinterpret_cast<FloatStore<N> &>(*this);
}
template<size_t N> requires(N <= 4)
FORCEINLINE FloatStore<4>::operator const FloatStore<N> &() const noexcept {
	return reinterpret_cast<const FloatStore<N> &>(*this);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// Swizzle


#if 1
template <typename T, bool EnableAssign, size_t... I>
template <typename> requires (EnableAssign)
FORCEINLINE T &Swizzle<T, EnableAssign, I...>::operator=(const T &other) noexcept {
	((at<I>() = other[I]), ...);
	constexpr size_t idx = indices[0];
	return reinterpret_cast<T &>(at<idx>());
}
template <typename T, bool EnableAssign, size_t... I>
FORCEINLINE Swizzle<T, EnableAssign, I...>::operator T() const noexcept {
	return T(at<I>()...);
}
template<typename T, bool EnableAssign, size_t... I>
FORCEINLINE std::ostream &operator<<(std::ostream &os, const Swizzle<T, EnableAssign, I...> &sz) noexcept {
	os << static_cast<T>(sz);
	return os;
}
template <typename T, bool EnableAssign, size_t... I>
template <size_t Idx>
FORCEINLINE float &Swizzle<T, EnableAssign, I...>::at() noexcept {
	return reinterpret_cast<float *>(this)[Idx];
}
template <typename T, bool EnableAssign, size_t... I>
template <size_t Idx>
FORCEINLINE float Swizzle<T, EnableAssign, I...>::at() const noexcept {
	return reinterpret_cast<const float *>(this)[Idx];
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// Scalar
#if 1
FORCEINLINE Scalar::Scalar(DX::XMVECTOR vec) noexcept : _vec(vec) {
}
FORCEINLINE Scalar::Scalar(float v) noexcept : _vec(DX::XMVectorSet(v, v, v, v)) {
}
FORCEINLINE Scalar::operator float() const noexcept {
	return  DX::XMVectorGetX(_vec);
}
FORCEINLINE DX::XMVECTOR *Scalar::operator&() noexcept {
	return &_vec;
}
FORCEINLINE const DX::XMVECTOR *Scalar::operator&() const noexcept {
	return &_vec;
}
FORCEINLINE Scalar::operator DX::XMVECTOR() const noexcept {
	return _vec;
}
FORCEINLINE Scalar operator- (Scalar s) noexcept {
	return Scalar(DX::XMVectorNegate(s));
}
FORCEINLINE Scalar operator+ (Scalar s1, Scalar s2) noexcept {
	return Scalar(DX::XMVectorAdd(s1, s2));
}
FORCEINLINE Scalar operator- (Scalar s1, Scalar s2) noexcept {
	return Scalar(DX::XMVectorSubtract(s1, s2));
}
FORCEINLINE Scalar operator* (Scalar s1, Scalar s2) noexcept {
	return Scalar(DX::XMVectorMultiply(s1, s2));
}
FORCEINLINE Scalar operator/ (Scalar s1, Scalar s2) noexcept {
	return Scalar(DX::XMVectorDivide(s1, s2));
}
FORCEINLINE Scalar operator+ (Scalar s1, float s2) noexcept {
	return s1 + Scalar(s2);
}
FORCEINLINE Scalar operator- (Scalar s1, float s2) noexcept {
	return s1 - Scalar(s2);
}
FORCEINLINE Scalar operator* (Scalar s1, float s2) noexcept {
	return s1 * Scalar(s2);
}
FORCEINLINE Scalar operator/ (Scalar s1, float s2) noexcept {
	return s1 / Scalar(s2);
}
FORCEINLINE Scalar operator+ (float s1, Scalar s2) noexcept {
	return Scalar(s1) + s2;
}
FORCEINLINE Scalar operator- (float s1, Scalar s2) noexcept {
	return Scalar(s1) - s2;
}
FORCEINLINE Scalar operator* (float s1, Scalar s2) noexcept {
	return Scalar(s1) * s2;
}
FORCEINLINE Scalar operator/ (float s1, Scalar s2) noexcept {
	return Scalar(s1) / s2;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Vector2
#if 1
FORCEINLINE Vector2::Vector2(float x, float y) noexcept : x(x), y(y) {
}
FORCEINLINE Vector2::Vector2(float val) noexcept : x(val), y(val) {
}
FORCEINLINE Vector2::Vector2(const float2 &f2) noexcept : x(f2.x), y(f2.y) {
}
FORCEINLINE Vector2::Vector2(const Vector3 &v3) noexcept : x(v3.x), y(v3.y) {
}
FORCEINLINE Vector2::Vector2(const Vector4 &v4) noexcept : x(v4.x), y(v4.y) {
}
FORCEINLINE Vector2 &Vector2::operator=(const Vector2 &other) noexcept {
	x = other.x;
	y = other.y;
	return *this;
}
FORCEINLINE float &Vector2::operator[](size_t index) noexcept {
	assert(index < 2);
	return reinterpret_cast<float *>(this)[index];
}
FORCEINLINE float Vector2::operator[](size_t index) const noexcept {
	assert(index < 2);
	return reinterpret_cast<const float *>(this)[index];
}

FORCEINLINE Vector2::operator float2() const noexcept {
	return xy;
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const Vector2 &v) noexcept {
	os << '(' << v.x << ", " << v.y << ')';
	return os;
}
FORCEINLINE Vector2 operator+(const Vector2 &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}
FORCEINLINE Vector2 operator-(const Vector2 &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
}
FORCEINLINE Vector2 operator*(const Vector2 &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs.x * rhs.x, lhs.y * rhs.y);
}
FORCEINLINE Vector2 operator/(const Vector2 &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs.x / rhs.x, lhs.y / rhs.y);
}
FORCEINLINE Vector2 operator+(const float &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs + rhs.x, lhs + rhs.y);
}
FORCEINLINE Vector2 operator-(const float &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs - rhs.x, lhs - rhs.y);
}
FORCEINLINE Vector2 operator*(const float &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs * rhs.x, lhs * rhs.y);
}
FORCEINLINE Vector2 operator/(const float &lhs, const Vector2 &rhs) noexcept {
	return Vector2(lhs / rhs.x, lhs / rhs.y);
}
FORCEINLINE Vector2 operator+(const Vector2 &lhs, const float &rhs) noexcept {
	return Vector2(lhs.x + rhs, lhs.y + rhs);
}
FORCEINLINE Vector2 operator-(const Vector2 &lhs, const float &rhs) noexcept {
	return Vector2(lhs.x - rhs, lhs.y - rhs);
}
FORCEINLINE Vector2 operator*(const Vector2 &lhs, const float &rhs) noexcept {
	return Vector2(lhs.x * rhs, lhs.y * rhs);
}
FORCEINLINE Vector2 operator/(const Vector2 &lhs, const float &rhs) noexcept {
	return Vector2(lhs.x / rhs, lhs.y / rhs);
}
FORCEINLINE Vector2 &operator+=(Vector2 &lhs, const Vector2 &rhs) noexcept {
	lhs = lhs + rhs;
	return lhs;
}
FORCEINLINE Vector2 &operator-=(Vector2 &lhs, const Vector2 &rhs) noexcept {
	lhs = lhs - rhs;
	return lhs;
}
FORCEINLINE Vector2 &operator*=(Vector2 &lhs, const Vector2 &rhs) noexcept {
	lhs = lhs * rhs;
	return lhs;
}
FORCEINLINE Vector2 &operator/=(Vector2 &lhs, const Vector2 &rhs) noexcept {
	lhs = lhs / rhs;
	return lhs;
}
FORCEINLINE Vector2 &operator+=(Vector2 &lhs, float rhs) noexcept {
	lhs = lhs + Vector2(rhs);
	return lhs;
}
FORCEINLINE Vector2 &operator-=(Vector2 &lhs, float rhs) noexcept {
	lhs = lhs - Vector2(rhs);
	return lhs;
}
FORCEINLINE Vector2 &operator*=(Vector2 &lhs, float rhs) noexcept {
	lhs = lhs * Vector2(rhs);
	return lhs;
}
FORCEINLINE Vector2 &operator/=(Vector2 &lhs, float rhs) noexcept {
	lhs = lhs / Vector2(rhs);
	return lhs;
}
FORCEINLINE Vector2 operator-(const Vector2 &v) noexcept {
	return Vector2(-v.x, -v.y);
}
FORCEINLINE Vector2 sqrt(const Vector2 &v) noexcept {
	return Vector2(std::sqrt(v.x), std::sqrt(v.y));
}
FORCEINLINE Vector2 inverse(const Vector2 &v) noexcept {
	return Vector2(1.f / v.x, 1.f / v.y);
}
FORCEINLINE Vector2 inverseSqrt(const Vector2 &v) noexcept {
	return Vector2(1.f / std::sqrt(v.x), 1.f / std::sqrt(v.y));
}
FORCEINLINE Vector2 floor(const Vector2 &v) noexcept {
	return Vector2(std::floor(v.x), std::floor(v.y));
}
FORCEINLINE Vector2 ceil(const Vector2 &v) noexcept {
	return Vector2(std::ceil(v.x), std::ceil(v.y));
}
FORCEINLINE Vector2 round(const Vector2 &v) noexcept {
	return Vector2(std::round(v.x), std::round(v.y));
}
FORCEINLINE Vector2 abs(const Vector2 &v) noexcept {
	return Vector2(std::abs(v.x), std::abs(v.y));
}
FORCEINLINE Vector2 exp(const Vector2 &v) noexcept {
	return Vector2(std::exp(v.x), std::exp(v.y));
}
FORCEINLINE Vector2 pow(const Vector2 &b, const Vector2 &e) noexcept {
	return Vector2(std::pow(b.x, e.x), std::pow(b.y, e.y));
}
FORCEINLINE Vector2 log(const Vector2 &v) noexcept {
	return Vector2(std::log(v.x), std::log(v.y));
}
FORCEINLINE Vector2 sin(const Vector2 &v) noexcept {
	return Vector2(std::sin(v.x), std::sin(v.y));
}
FORCEINLINE Vector2 cos(const Vector2 &v) noexcept {
	return Vector2(std::cos(v.x), std::cos(v.y));
}
FORCEINLINE Vector2 tan(const Vector2 &v) noexcept {
	return Vector2(std::tan(v.x), std::tan(v.y));
}
FORCEINLINE Vector2 asin(const Vector2 &v) noexcept {
	return Vector2(std::asin(v.x), std::asin(v.y));
}
FORCEINLINE Vector2 acos(const Vector2 &v) noexcept {
	return Vector2(std::acos(v.x), std::acos(v.y));
}
FORCEINLINE Vector2 atan(const Vector2 &v) noexcept {
	return Vector2(std::atan(v.x), std::atan(v.y));
}
FORCEINLINE Vector2 atan2(const Vector2 y, const Vector2 x) noexcept {
	return Vector2(std::atan2(y.x, x.x), std::atan2(y.y, x.y));
}
FORCEINLINE Vector2 lerp(const Vector2 &a, const Vector2 &b, const Vector2 &t) noexcept {
	return Vector2(
		(a.x * (1.f - t.x)) + b.x * t.x,
		(a.y * (1.f - t.y)) + b.y * t.y
	);
}
FORCEINLINE Vector2 lerp(const Vector2 &a, const Vector2 &b, float t) noexcept {
	return Vector2(
		(a.x * (1.f - t)) + b.x * t,
		(a.y * (1.f - t)) + b.y * t
	);
}
FORCEINLINE Vector2 max(const Vector2 &a, const Vector2 &b) noexcept {
	return Vector2(
		std::max(a.x, b.x),
		std::max(a.y, b.y)
	);
}
FORCEINLINE Vector2 min(const Vector2 &a, const Vector2 &b) noexcept {
	return Vector2(
		std::min(a.x, b.x),
		std::min(a.y, b.y)
	);
}
FORCEINLINE Vector2 clamp(const Vector2 &v, const Vector2 &a, const Vector2 &b) noexcept {
	return Vector2(
		std::clamp(v.x, a.x, b.x),
		std::clamp(v.y, a.y, b.y)
	);
}
FORCEINLINE float dot(const Vector2 &v1, const Vector2 &v2) noexcept {
	return v1.x * v2.x + v1.y * v2.y;
}
FORCEINLINE float lengthSquare(const Vector2 &v) {
	return v.x * v.x + v.y * v.y;
}
FORCEINLINE float length(const Vector2 &v) {
	float lengthSqr = lengthSquare(v);
	if (lengthSqr > 0.f)
		return std::sqrt(lengthSqr);
	else
		return 0.f;
}
FORCEINLINE float inverseLength(const Vector2 &v) {
	return 1.f / length(v);
}
FORCEINLINE Vector2 normalize(const Vector2 &v) noexcept {
	float lengthSqr = lengthSquare(v);;
	if (lengthSqr > 0.f)
		return v / std::sqrt(lengthSqr);
	else
		return Vector2(0.f);
}
#endif
/// Vectror3
#if 1
FORCEINLINE Vector3 operator+(const Vector3 &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorAdd(lhs, rhs);
}
FORCEINLINE Vector3 operator-(const Vector3 &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, rhs);
}
FORCEINLINE Vector3 operator*(const Vector3 &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, rhs);
}
FORCEINLINE Vector3 operator/(const Vector3 &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorDivide(lhs, rhs);
}
FORCEINLINE Vector3 operator+(const Vector3 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorAdd(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector3 operator-(const Vector3 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector3 operator*(const Vector3 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector3 operator/(const Vector3 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorDivide(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector3 operator+(const Scalar &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorAdd(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector3 operator-(const Scalar &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorSubtract(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector3 operator*(const Scalar &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorMultiply(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector3 operator/(const Scalar &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorDivide(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector3 operator+(const float &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorAdd(Vector3(lhs), rhs);
}
FORCEINLINE Vector3 operator-(const float &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorSubtract(Vector3(lhs), rhs);
}
FORCEINLINE Vector3 operator*(const float &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorMultiply(Vector3(lhs), rhs);
}
FORCEINLINE Vector3 operator/(const float &lhs, const Vector3 &rhs) noexcept {
	return DX::XMVectorDivide(Vector3(lhs), rhs);
}
FORCEINLINE Vector3 operator+(const Vector3 &lhs, const float &rhs) noexcept {
	return DX::XMVectorAdd(lhs, Vector3(rhs));
}
FORCEINLINE Vector3 operator-(const Vector3 &lhs, const float &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, Vector3(rhs));
}
FORCEINLINE Vector3 operator*(const Vector3 &lhs, const float &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, Vector3(rhs));
}
FORCEINLINE Vector3 operator/(const Vector3 &lhs, const float &rhs) noexcept {
	return DX::XMVectorDivide(lhs, Vector3(rhs));
}
FORCEINLINE Vector3 &operator+=(Vector3 &lhs, const Vector3 &rhs) noexcept {
	lhs = lhs + rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator-=(Vector3 &lhs, const Vector3 &rhs) noexcept {
	lhs = lhs - rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator*=(Vector3 &lhs, const Vector3 &rhs) noexcept {
	lhs = lhs * rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator/=(Vector3 &lhs, const Vector3 &rhs) noexcept {
	lhs = lhs / rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator+=(Vector3 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs + rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator-=(Vector3 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs - rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator*=(Vector3 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs * rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator/=(Vector3 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs / rhs;
	return lhs;
}
FORCEINLINE Vector3 &operator+=(Vector3 &lhs, float rhs) noexcept {
	lhs = lhs + Vector3(rhs);
	return lhs;
}
FORCEINLINE Vector3 &operator-=(Vector3 &lhs, float rhs) noexcept {
	lhs = lhs - Vector3(rhs);
	return lhs;
}
FORCEINLINE Vector3 &operator*=(Vector3 &lhs, float rhs) noexcept {
	lhs = lhs * Vector3(rhs);
	return lhs;
}
FORCEINLINE Vector3 &operator/=(Vector3 &lhs, float rhs) noexcept {
	lhs = lhs / Vector3(rhs);
	return lhs;
}
FORCEINLINE Vector3 operator-(const Vector3 &v) noexcept {
	return DX::XMVectorNegate(v);
}
FORCEINLINE BoolVector operator<  (Vector3 lhs, Vector3 rhs) noexcept {
	return DX::XMVectorLess(lhs, rhs);
}
FORCEINLINE BoolVector operator<= (Vector3 lhs, Vector3 rhs) noexcept {
	return DX::XMVectorLessOrEqual(lhs, rhs);
}
FORCEINLINE BoolVector operator>  (Vector3 lhs, Vector3 rhs) noexcept {
	return DX::XMVectorGreater(lhs, rhs);
}
FORCEINLINE BoolVector operator>= (Vector3 lhs, Vector3 rhs) noexcept {
	return DX::XMVectorGreaterOrEqual(lhs, rhs);
}
FORCEINLINE BoolVector operator== (Vector3 lhs, Vector3 rhs) noexcept {
	return DX::XMVectorEqual(lhs, rhs);
}
FORCEINLINE Vector3 sqrt(Vector3 s) noexcept {
	return Vector3(DX::XMVectorSqrt(s));
}
FORCEINLINE Vector3 inverse(Vector3 s) noexcept {
	return Vector3(DX::XMVectorReciprocal(s));
}
FORCEINLINE Vector3 inverseSqrt(Vector3 s) noexcept {
	return Vector3(DX::XMVectorReciprocalSqrt(s));
}
FORCEINLINE Vector3 floor(Vector3 s) noexcept {
	return Vector3(DX::XMVectorFloor(s));
}
FORCEINLINE Vector3 ceil(Vector3 s) noexcept {
	return Vector3(DX::XMVectorCeiling(s));
}
FORCEINLINE Vector3 round(Vector3 s) noexcept {
	return Vector3(DX::XMVectorRound(s));
}
FORCEINLINE Vector3 abs(Vector3 s) noexcept {
	return Vector3(DX::XMVectorAbs(s));
}
FORCEINLINE Vector3 exp(Vector3 s) noexcept {
	return Vector3(DX::XMVectorExp(s));
}
FORCEINLINE Vector3 pow(Vector3 b, Vector3 e) noexcept {
	return Vector3(DX::XMVectorPow(b, e));
}
FORCEINLINE Vector3 log(Vector3 s) noexcept {
	return Vector3(DX::XMVectorLog(s));
}
FORCEINLINE Vector3 sin(Vector3 s) noexcept {
	return Vector3(DX::XMVectorSin(s));
}
FORCEINLINE Vector3 cos(Vector3 s) noexcept {
	return Vector3(DX::XMVectorCos(s));
}
FORCEINLINE Vector3 tan(Vector3 s) noexcept {
	return Vector3(DX::XMVectorTan(s));
}
FORCEINLINE Vector3 asin(Vector3 s) noexcept {
	return Vector3(DX::XMVectorASin(s));
}
FORCEINLINE Vector3 acos(Vector3 s) noexcept {
	return Vector3(DX::XMVectorACos(s));
}
FORCEINLINE Vector3 atan(Vector3 s) noexcept {
	return Vector3(DX::XMVectorATan(s));
}
FORCEINLINE Vector3 atan2(Vector3 y, Vector3 x) noexcept {
	return Vector3(DX::XMVectorATan2(y, x));
}
FORCEINLINE Vector3 lerp(Vector3 a, Vector3 b, Vector3 t) noexcept {
	return Vector3(DX::XMVectorLerpV(a, b, t));
}
FORCEINLINE Vector3 lerp(Vector3 a, Vector3 b, float t) noexcept {
	return Vector3(DX::XMVectorLerp(a, b, t));
}
FORCEINLINE Vector3 max(Vector3 a, Vector3 b) noexcept {
	return Vector3(DX::XMVectorMax(a, b));
}
FORCEINLINE Vector3 min(Vector3 a, Vector3 b) noexcept {
	return Vector3(DX::XMVectorMin(a, b));
}
FORCEINLINE Vector3 clamp(Vector3 v, Vector3 a, Vector3 b) noexcept {
	return min(max(v, a), b);
}
FORCEINLINE Vector3 normalize(const Vector3 &v) noexcept {
	return DX::XMVector3Normalize(v);
}
FORCEINLINE Scalar dot(const Vector3 &v1, const Vector3 &v2) noexcept {
	return Scalar(DX::XMVector3Dot(v1, v2));
}
FORCEINLINE Vector3 cross(const Vector3 &v0, const Vector3 &v1) {
	return Scalar(DX::XMVector3Cross(v0, v1));
}
FORCEINLINE Scalar length(const Vector3 &v) {
	return Scalar(DX::XMVector3Length(v));
}
FORCEINLINE Scalar lengthSquare(const Vector3 &v) {
	return Scalar(DX::XMVector3LengthSq(v));
}
FORCEINLINE Scalar inverseLength(const Vector3 &v) {
	return Scalar(DX::XMVector3ReciprocalLength(v));
}
FORCEINLINE Vector3::Vector3(const float2 &f2, float v) noexcept : Vector3(f2.x, f2.y, v) {
}

FORCEINLINE Vector3::Vector3(const float3 &f3) noexcept : Vector3(f3.x, f3.y, f3.z) {
}
FORCEINLINE Vector3::Vector3(DX::XMVECTOR vec) noexcept : _vec(vec) {
}
FORCEINLINE Vector3::Vector3(const Scalar &s) noexcept : _vec(s) {
}
FORCEINLINE Vector3::Vector3(float x, float y, float z) noexcept {
	_vec = DX::XMVectorSet(x, y, z, 0.0);
}
FORCEINLINE Vector3::Vector3(float val) noexcept {
	_vec = DX::XMVectorSet(val, val, val, 0.0);
}
FORCEINLINE Vector3 &Vector3::operator=(const Vector3 &other) noexcept {
	_vec = other._vec;
	return *this;
}
FORCEINLINE float &Vector3::operator[](size_t index) noexcept {
	assert(index < 3);
	return _vec.m128_f32[index];
}
FORCEINLINE float Vector3::operator[](size_t index) const noexcept {
	assert(index < 3);
	return _vec.m128_f32[index];
}
FORCEINLINE DX::XMVECTOR *Vector3::operator&() noexcept {
	return &_vec;
}
FORCEINLINE const DX::XMVECTOR *Vector3::operator&() const noexcept {
	return &_vec;
}
FORCEINLINE Vector3::operator DX::XMVECTOR() const noexcept {
	return _vec;
}

FORCEINLINE Vector3::operator float2() const noexcept {
	return xy;
}

FORCEINLINE Vector3::operator float3() const noexcept {
	return xyz;
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const Vector3 &v) noexcept {
	os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
	return os;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// Vectror4
#if 1
FORCEINLINE Vector4 operator+(const Vector4 &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorAdd(lhs, rhs);
}
FORCEINLINE Vector4 operator-(const Vector4 &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, rhs);
}
FORCEINLINE Vector4 operator*(const Vector4 &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, rhs);
}
FORCEINLINE Vector4 operator/(const Vector4 &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorDivide(lhs, rhs);
}
FORCEINLINE Vector4 operator+(const Vector4 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorAdd(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector4 operator-(const Vector4 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector4 operator*(const Vector4 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector4 operator/(const Vector4 &lhs, const Scalar &rhs) noexcept {
	return DX::XMVectorDivide(lhs, DX::XMVECTOR(rhs));
}
FORCEINLINE Vector4 operator+(const Scalar &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorAdd(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector4 operator-(const Scalar &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorSubtract(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector4 operator*(const Scalar &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorMultiply(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector4 operator/(const Scalar &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorDivide(DX::XMVECTOR(lhs), rhs);
}
FORCEINLINE Vector4 operator+(const float &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorAdd(Vector4(lhs), rhs);
}
FORCEINLINE Vector4 operator-(const float &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorSubtract(Vector4(lhs), rhs);
}
FORCEINLINE Vector4 operator*(const float &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorMultiply(Vector4(lhs), rhs);
}
FORCEINLINE Vector4 operator/(const float &lhs, const Vector4 &rhs) noexcept {
	return DX::XMVectorDivide(Vector4(lhs), rhs);
}
FORCEINLINE Vector4 operator+(const Vector4 &lhs, const float &rhs) noexcept {
	return DX::XMVectorAdd(lhs, Vector4(rhs));
}
FORCEINLINE Vector4 operator-(const Vector4 &lhs, const float &rhs) noexcept {
	return DX::XMVectorSubtract(lhs, Vector4(rhs));
}
FORCEINLINE Vector4 operator*(const Vector4 &lhs, const float &rhs) noexcept {
	return DX::XMVectorMultiply(lhs, Vector4(rhs));
}
FORCEINLINE Vector4 operator/(const Vector4 &lhs, const float &rhs) noexcept {
	return DX::XMVectorDivide(lhs, Vector4(rhs));
}
FORCEINLINE Vector4 &operator+=(Vector4 &lhs, const Vector4 &rhs) noexcept {
	lhs = lhs + rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator-=(Vector4 &lhs, const Vector4 &rhs) noexcept {
	lhs = lhs - rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator*=(Vector4 &lhs, const Vector4 &rhs) noexcept {
	lhs = lhs * rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator/=(Vector4 &lhs, const Vector4 &rhs) noexcept {
	lhs = lhs / rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator+=(Vector4 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs + rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator-=(Vector4 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs - rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator*=(Vector4 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs * rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator/=(Vector4 &lhs, const Scalar &rhs) noexcept {
	lhs = lhs / rhs;
	return lhs;
}
FORCEINLINE Vector4 &operator+=(Vector4 &lhs, float rhs) noexcept {
	lhs = lhs + Vector4(rhs);
	return lhs;
}
FORCEINLINE Vector4 &operator-=(Vector4 &lhs, float rhs) noexcept {
	lhs = lhs - Vector4(rhs);
	return lhs;
}
FORCEINLINE Vector4 &operator*=(Vector4 &lhs, float rhs) noexcept {
	lhs = lhs * Vector4(rhs);
	return lhs;
}
FORCEINLINE Vector4 &operator/=(Vector4 &lhs, float rhs) noexcept {
	lhs = lhs / Vector4(rhs);
	return lhs;
}
FORCEINLINE Vector4 operator-(const Vector4 &v) noexcept {
	return DX::XMVectorNegate(v);
}
FORCEINLINE BoolVector operator<  (Vector4 lhs, Vector4 rhs) noexcept {
	return DX::XMVectorLess(lhs, rhs);
}
FORCEINLINE BoolVector operator<= (Vector4 lhs, Vector4 rhs) noexcept {
	return DX::XMVectorLessOrEqual(lhs, rhs);
}
FORCEINLINE BoolVector operator>  (Vector4 lhs, Vector4 rhs) noexcept {
	return DX::XMVectorGreater(lhs, rhs);
}
FORCEINLINE BoolVector operator>= (Vector4 lhs, Vector4 rhs) noexcept {
	return DX::XMVectorGreaterOrEqual(lhs, rhs);
}
FORCEINLINE BoolVector operator== (Vector4 lhs, Vector4 rhs) noexcept {
	return DX::XMVectorEqual(lhs, rhs);
}
FORCEINLINE Vector4 sqrt(Vector4 s) noexcept {
	return Vector4(DX::XMVectorSqrt(s));
}
FORCEINLINE Vector4 inverse(Vector4 s) noexcept {
	return Vector4(DX::XMVectorReciprocal(s));
}
FORCEINLINE Vector4 inverseSqrt(Vector4 s) noexcept {
	return Vector4(DX::XMVectorReciprocalSqrt(s));
}
FORCEINLINE Vector4 floor(Vector4 s) noexcept {
	return Vector4(DX::XMVectorFloor(s));
}
FORCEINLINE Vector4 ceil(Vector4 s) noexcept {
	return Vector4(DX::XMVectorCeiling(s));
}
FORCEINLINE Vector4 round(Vector4 s) noexcept {
	return Vector4(DX::XMVectorRound(s));
}
FORCEINLINE Vector4 abs(Vector4 s) noexcept {
	return Vector4(DX::XMVectorAbs(s));
}
FORCEINLINE Vector4 exp(Vector4 s) noexcept {
	return Vector4(DX::XMVectorExp(s));
}
FORCEINLINE Vector4 pow(Vector4 b, Vector4 e) noexcept {
	return Vector4(DX::XMVectorPow(b, e));
}
FORCEINLINE Vector4 log(Vector4 s) noexcept {
	return Vector4(DX::XMVectorLog(s));
}
FORCEINLINE Vector4 sin(Vector4 s) noexcept {
	return Vector4(DX::XMVectorSin(s));
}
FORCEINLINE Vector4 cos(Vector4 s) noexcept {
	return Vector4(DX::XMVectorCos(s));
}
FORCEINLINE Vector4 tan(Vector4 s) noexcept {
	return Vector4(DX::XMVectorTan(s));
}
FORCEINLINE Vector4 asin(Vector4 s) noexcept {
	return Vector4(DX::XMVectorASin(s));
}
FORCEINLINE Vector4 acos(Vector4 s) noexcept {
	return Vector4(DX::XMVectorACos(s));
}
FORCEINLINE Vector4 atan(Vector4 s) noexcept {
	return Vector4(DX::XMVectorATan(s));
}
FORCEINLINE Vector4 atan2(Vector4 y, Vector4 x) noexcept {
	return Vector4(DX::XMVectorATan2(y, x));
}
FORCEINLINE Vector4 lerp(Vector4 a, Vector4 b, Vector4 t) noexcept {
	return Vector4(DX::XMVectorLerpV(a, b, t));
}
FORCEINLINE Vector4 lerp(Vector4 a, Vector4 b, float t) noexcept {
	return Vector4(DX::XMVectorLerp(a, b, t));
}
FORCEINLINE Vector4 max(Vector4 a, Vector4 b) noexcept {
	return Vector4(DX::XMVectorMax(a, b));
}
FORCEINLINE Vector4 min(Vector4 a, Vector4 b) noexcept {
	return Vector4(DX::XMVectorMin(a, b));
}
FORCEINLINE Vector4 clamp(Vector4 v, Vector4 a, Vector4 b) noexcept {
	return min(max(v, a), b);
}
FORCEINLINE Scalar dot(const Vector4 & v1, const Vector4 & v2) noexcept {
	return Scalar(DX::XMVector4Dot(v1, v2));
}
FORCEINLINE Scalar length(const Vector4 & v) {
	return Scalar(DX::XMVector4Length(v));
}
FORCEINLINE Scalar lengthSquare(const Vector4 & v) {
	return Scalar(DX::XMVector4LengthSq(v));
}
FORCEINLINE Scalar inverseLength(const Vector4 & v) {
	return Scalar(DX::XMVector4ReciprocalLength(v));
}
FORCEINLINE Vector4::Vector4(const float3 &f3, float v) noexcept : Vector4(f3.x, f3.y, f3.z, v) {
}
FORCEINLINE Vector4::Vector4(const float4 &f4) noexcept : Vector4(f4.x, f4.y, f4.z, f4.w) {
}
FORCEINLINE Vector4::Vector4(DX::XMVECTOR vec) noexcept : vec(vec) {
}
FORCEINLINE Vector4::Vector4(const Scalar &s) noexcept {
	vec = DX::XMVECTOR(s);
}
FORCEINLINE Vector4::Vector4(float x, float y, float z, float w) noexcept {
	vec = DX::XMVectorSet(x, y, z, w);
}
FORCEINLINE Vector4::Vector4(float val) noexcept {
	vec = DX::XMVectorSet(val, val, val, val);
}
FORCEINLINE Vector4 &Vector4::operator=(const Vector4 &other) noexcept {
	vec = other.vec;
	return *this;
}
FORCEINLINE float &Vector4::operator[](size_t index) noexcept {
	assert(index < 4);
	return vec.m128_f32[index];
}
FORCEINLINE float Vector4::operator[](size_t index) const noexcept {
	assert(index < 4);
	return vec.m128_f32[index];
}
FORCEINLINE DX::XMVECTOR *Vector4::operator&() noexcept {
	return &vec;
}
FORCEINLINE const DX::XMVECTOR *Vector4::operator&() const noexcept {
	return &vec;
}
Vector4::operator DX::XMVECTOR() const noexcept {
	return vec;
}

FORCEINLINE Vector4::operator float2() const noexcept {
	return xy;
}

FORCEINLINE Vector4::operator float3() const noexcept {
	return xyz;
}
FORCEINLINE Vector4::operator float4() const noexcept {
	return xyzw;
}
FORCEINLINE std::ostream &operator<<(std::ostream &os, const Vector4 &v) noexcept {
	os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
	return os;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// BoolVector
#if 1
FORCEINLINE BoolVector::BoolVector(DX::FXMVECTOR vec) noexcept : _vec(vec) {
}
FORCEINLINE BoolVector::operator DX::XMVECTOR() const {
	return _vec;
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// Matrix3x3
#if 1
FORCEINLINE Matrix3::Matrix3(const float3x3 &f3x3) noexcept {
	_mat[0] = Vector3(f3x3.m[0][0], f3x3.m[0][1], f3x3.m[0][2]);
	_mat[1] = Vector3(f3x3.m[1][0], f3x3.m[1][1], f3x3.m[1][2]);
	_mat[2] = Vector3(f3x3.m[2][0], f3x3.m[2][1], f3x3.m[2][2]);
}

FORCEINLINE Matrix3::Matrix3(const DX::XMMATRIX &mat) noexcept {
	_mat[0] = Vector3(mat.r[0]);
	_mat[1] = Vector3(mat.r[1]);
	_mat[2] = Vector3(mat.r[2]);
}
FORCEINLINE Matrix3::Matrix3(const Matrix4 &mat4) noexcept : Matrix3(static_cast<DX::XMMATRIX>(mat4)) {
}
FORCEINLINE Matrix3::Matrix3(const Vector3 &x, const Vector3 &y, const Vector3 &z) noexcept {
	_mat[0] = x;
	_mat[1] = y;
	_mat[2] = z;
}
FORCEINLINE void Matrix3::setX(const Vector3 &x) noexcept {
	_mat[0] = x;
}
FORCEINLINE void Matrix3::setY(const Vector3 &y) noexcept {
	_mat[1] = y;
}
FORCEINLINE void Matrix3::setZ(const Vector3 &z) noexcept {
	_mat[2] = z;
}
FORCEINLINE Vector3 Matrix3::getX() const noexcept {
	return _mat[0];
}
FORCEINLINE Vector3 Matrix3::getY() const noexcept {
	return _mat[1];
}
FORCEINLINE Vector3 Matrix3::getZ() const noexcept {
	return _mat[2];
}
FORCEINLINE Matrix3::operator DX::XMMATRIX() const noexcept {
	return DX::XMMATRIX(getX(), getY(), getZ(), Vector4(0.f, 0.f, 0.f, 1.f));
}
FORCEINLINE Matrix3::operator Matrix4() const noexcept {
	return Matrix4(static_cast<DX::XMMATRIX>(*this));
}
FORCEINLINE Matrix3::operator float3x3() const noexcept {
	return float3x3(
		_mat[0].x, _mat[0].y, _mat[0].z,
		_mat[1].x, _mat[1].y, _mat[1].z,
		_mat[2].x, _mat[2].y, _mat[2].z
	);
}
FORCEINLINE Vector3 &Matrix3::operator[](size_t index) noexcept {
	assert(index < 3);
	return _mat[index];
}
FORCEINLINE const Vector3 &Matrix3::operator[](size_t index) const noexcept {
	assert(index < 3);
	return _mat[index];
}
FORCEINLINE Matrix3 Matrix3::operator*(const Scalar &s) const noexcept {
	return Matrix3(
		getX() * s,
		getY() * s,
		getZ() * s
	);
}
FORCEINLINE Vector3 Matrix3::operator*(const Vector3 &v) const noexcept {
	return Vector3(DX::XMVector3TransformNormal(v, this->operator DX::XMMATRIX()));
}
FORCEINLINE Matrix3 Matrix3::operator*(const Matrix3 &mat) const noexcept {
	return Matrix3(
		*this * mat.getX(), 
		*this * mat.getY(), 
		*this * mat.getZ()
	);
}
FORCEINLINE Matrix3 Matrix3::makeZRotationByRadian(float radian) noexcept {
	return Matrix3(DX::XMMatrixRotationZ(radian));
}
FORCEINLINE Matrix3 Matrix3::makeXRotationByRadian(float radian) noexcept {
	return Matrix3(DX::XMMatrixRotationX(radian));
}
FORCEINLINE Matrix3 Matrix3::makeYRotationByRadian(float radian) noexcept {
	return Matrix3(DX::XMMatrixRotationY(radian));
}
FORCEINLINE Matrix3 Matrix3::makeZRotationByDegree(float angle) noexcept {
	return Matrix3(DX::XMMatrixRotationZ(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix3 Matrix3::makeXRotationByDegree(float angle) noexcept {
	return Matrix3(DX::XMMatrixRotationX(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix3 Matrix3::makeYRotationByDegree(float angle) noexcept {
	return Matrix3(DX::XMMatrixRotationY(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix3 Matrix3::makeAxisRotationByDegree(const Vector3 &axis, float angle) noexcept {
	return Matrix3(DX::XMMatrixRotationAxis(axis, DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix3 Matrix3::makeAxisRotationByRadian(const Vector3 &axis, float radian) noexcept {
	return Matrix3(DX::XMMatrixRotationAxis(axis, radian));
}
FORCEINLINE Matrix3 Matrix3::makeScale(float scale) noexcept {
	return Matrix3(DX::XMMatrixScaling(scale, scale, scale));
}
FORCEINLINE Matrix3 Matrix3::makeScale(float sx, float sy, float sz) noexcept {
	return Matrix3(DX::XMMatrixScaling(sx, sy, sz));
}
FORCEINLINE Matrix3 Matrix3::makeScale(const float3 &scale) noexcept {
	return Matrix3(DX::XMMatrixScaling(scale.x, scale.y, scale.z));
}
FORCEINLINE Matrix3 Matrix3::makeScale(const Vector3 &scale) noexcept {
	return Matrix3(DX::XMMatrixScaling(scale.x, scale.y, scale.z));
}
FORCEINLINE Matrix3 Matrix3::identity() noexcept {
	static Matrix3 identity(Vector3(1.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f));
	return identity;
}
FORCEINLINE Matrix3 transpose(const Matrix3 &mat3) noexcept {
	return Matrix3(DX::XMMatrixTranspose(static_cast<DX::XMMATRIX>(mat3)));
}
FORCEINLINE Scalar determinant(const Matrix3 &mat3) noexcept {
	return DX::XMMatrixDeterminant(static_cast<DX::XMMATRIX>(mat3));
}
FORCEINLINE Matrix3 inverse(const Matrix3 &mat3, const Scalar &det) noexcept {
	Scalar d = det;
	return Matrix3(DX::XMMatrixInverse(&d, static_cast<DX::XMMATRIX>(mat3)));
}
FORCEINLINE Matrix3 inverse(const Matrix3 &mat3) noexcept {
	Scalar det = determinant(mat3);
	return Matrix3(DX::XMMatrixInverse(&det, static_cast<DX::XMMATRIX>(mat3)));
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////
/// Matrix4Сx4
#if 1
FORCEINLINE Matrix4::Matrix4(const DX::XMMATRIX &mat) noexcept : _mat(mat) {
}
FORCEINLINE Matrix4::Matrix4(const float4x4 &f4x4) noexcept {
	_mat = DX::XMLoadFloat4x4(&f4x4);
}
FORCEINLINE Matrix4::Matrix4(const Matrix3 &mat3) noexcept {
	_mat = mat3.operator DX::XMMATRIX();
}
FORCEINLINE Matrix4::Matrix4(const float *data) noexcept {
	_mat = DX::XMLoadFloat4x4(reinterpret_cast<const DX::XMFLOAT4X4 *>(data));
}
FORCEINLINE Matrix4::Matrix4(const Vector3 &x, const Vector3 &y, const Vector3 &z, const Vector3 &w) noexcept {
	_mat.r[0] = x;
	_mat.r[1] = y;
	_mat.r[2] = z;
	_mat.r[3] = w;
}
FORCEINLINE Matrix4::Matrix4(const Vector4 &x, const Vector4 &y, const Vector4 &z, const Vector4 &w) noexcept {
	_mat.r[0] = x;
	_mat.r[1] = y;
	_mat.r[2] = z;
	_mat.r[3] = w;
}
FORCEINLINE const Matrix3 &Matrix4::get3x3() const noexcept {
	return reinterpret_cast<const Matrix3 &>(*this);
}
FORCEINLINE void Matrix4::set3x3(const Matrix3 &xyz) noexcept {
	_mat.r[0] = xyz.getX();
	_mat.r[1] = xyz.getY();
	_mat.r[2] = xyz.getZ();
}
FORCEINLINE Vector4 Matrix4::getX() const noexcept {
	return _mat.r[0];
}
FORCEINLINE Vector4 Matrix4::getY() const noexcept {
	return _mat.r[1];
}
FORCEINLINE Vector4 Matrix4::getZ() const noexcept {
	return _mat.r[2];
}
FORCEINLINE Vector4 Matrix4::getW() const noexcept {
	return _mat.r[3];
}
FORCEINLINE void Matrix4::setX(Vector4 x) noexcept {
	_mat.r[0] = x;
}
FORCEINLINE void Matrix4::setY(Vector4 y) noexcept {
	_mat.r[1] = y;
}
FORCEINLINE void Matrix4::setZ(Vector4 z) noexcept {
	_mat.r[2] = z;
}
FORCEINLINE void Matrix4::setW(Vector4 w) noexcept {
	_mat.r[3] = w;
}
FORCEINLINE Matrix4::operator DirectX::XMMATRIX() const noexcept {
	return _mat;
}
FORCEINLINE Matrix4::operator float3x3() const noexcept {
	float3x3 res;
	DX::XMStoreFloat3x3(&res, _mat);
	return res;
}
FORCEINLINE Matrix4::operator float4x3() const noexcept {
	float4x3 res;
	DX::XMStoreFloat4x3(&res, _mat);
	return res;
}
FORCEINLINE Matrix4::operator DirectX::XMFLOAT4X4() const noexcept {
	float4x4 res;
	DX::XMStoreFloat4x4(&res, _mat);
	return res;
}
FORCEINLINE DX::XMMATRIX *Matrix4::operator&() noexcept {
	return &_mat;
}
FORCEINLINE const DX::XMMATRIX *Matrix4::operator&() const noexcept {
	return &_mat;
}
FORCEINLINE Vector4 Matrix4::operator*(const Vector3 &vec) const noexcept {
	return Vector4(DX::XMVector3Transform(vec, _mat));
}
FORCEINLINE Vector4 Matrix4::operator*(const Vector4 &vec) const noexcept {
	return Vector4(DX::XMVector3Transform(vec, _mat));
}
FORCEINLINE Matrix4 Matrix4::operator*(const Matrix4 &mat) const noexcept {
	return Matrix4(DX::XMMatrixMultiply(static_cast<DX::XMMATRIX>(mat), _mat));
}
FORCEINLINE Matrix4 Matrix4::makeZRotationByRadian(float radian) noexcept {
	return Matrix4(DX::XMMatrixRotationZ(radian));
}
FORCEINLINE Matrix4 Matrix4::makeXRotationByRadian(float radian) noexcept {
	return Matrix4(DX::XMMatrixRotationX(radian));
}
FORCEINLINE Matrix4 Matrix4::makeYRotationByRadian(float radian) noexcept {
	return Matrix4(DX::XMMatrixRotationY(radian));
}
FORCEINLINE Matrix4 Matrix4::makeZRotationByDegree(float angle) noexcept {
	return Matrix4(DX::XMMatrixRotationZ(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix4 Matrix4::makeXRotationByDegree(float angle) noexcept {
	return Matrix4(DX::XMMatrixRotationX(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix4 Matrix4::makeYRotationByDegree(float angle) noexcept {
	return Matrix4(DX::XMMatrixRotationY(DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix4 Matrix4::makeAxisRotationByDegree(const Vector3 &axis, float angle) noexcept {
	return Matrix4(DX::XMMatrixRotationAxis(axis, DX::XMConvertToRadians(angle)));
}
FORCEINLINE Matrix4 Matrix4::makeAxisRotationByRadian(const Vector3 &axis, float radian) noexcept {
	return Matrix4(DX::XMMatrixRotationAxis(axis, radian));
}
FORCEINLINE Matrix4 Matrix4::makeScale(float scale) noexcept {
	return Matrix4(DX::XMMatrixScaling(scale, scale, scale));
}
FORCEINLINE Matrix4 Matrix4::makeScale(float sx, float sy, float sz) noexcept {
	return Matrix4(DX::XMMatrixScaling(sx, sy, sz));
}
FORCEINLINE Matrix4 Matrix4::makeScale(const float3 &scale) noexcept {
	return Matrix4(DX::XMMatrixScaling(scale.x, scale.y, scale.z));
}
FORCEINLINE Matrix4 Matrix4::makeScale(const Vector3 &scale) noexcept {
	return Matrix4(DX::XMMatrixScaling(scale.x, scale.y, scale.z));
}
FORCEINLINE Matrix4 Matrix4::makeTranslation(const Vector3 &vec) noexcept {
	return Matrix4(DX::XMMatrixTranslation(vec.x, vec.y, vec.z));
}
FORCEINLINE Matrix4 Matrix4::makeTranslation(float ox, float oy, float oz) noexcept {
	return Matrix4(DX::XMMatrixTranslation(ox, oy, oz));
}
FORCEINLINE Matrix4 Matrix4::identity() noexcept {
	return DX::XMMatrixIdentity();
}
FORCEINLINE Matrix4 transpose(const Matrix4 &mat4) noexcept {
	return Matrix4(DX::XMMatrixTranspose(static_cast<DX::XMMATRIX>(mat4)));
}
FORCEINLINE Scalar determinant(const Matrix4 &mat4) noexcept {
	return DX::XMMatrixDeterminant(static_cast<DX::XMMATRIX>(mat4));
}
FORCEINLINE Matrix4 inverse(const Matrix4 &mat4, const Scalar &det) noexcept {
	Scalar d = det;
	return Matrix4(DX::XMMatrixInverse(&d, static_cast<DX::XMMATRIX>(mat4)));
}
FORCEINLINE Matrix4 inverse(const Matrix4 &mat4) noexcept {
	Scalar det = determinant(mat4);
	return Matrix4(DX::XMMatrixInverse(&det, static_cast<DX::XMMATRIX>(mat4)));
}
#endif
}


static_assert(sizeof(Math::float2) == sizeof(float)*2);
static_assert(sizeof(Math::float3) == sizeof(float)*3);
static_assert(sizeof(Math::float4) == sizeof(float)*4);