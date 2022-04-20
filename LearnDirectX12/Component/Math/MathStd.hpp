#pragma once
#define NOMINMAX
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
class Vector2;
class Vector3;
class Vector4;

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
	FORCEINLINE FloatStore(const Vector3 &vec) noexcept;
	FORCEINLINE FloatStore(const Vector4 &vec) noexcept;
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
	Swizzle() noexcept = default;
	template<typename = void> requires(EnableAssign)
	FORCEINLINE T &operator=(const T &other) noexcept;
	FORCEINLINE operator T() const noexcept;
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Swizzle &sz) noexcept;
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


//////////////////////////////////////////////////////////////////////////////////////////////////////
FORCEINLINE FloatStore<2>::FloatStore(DX::FXMVECTOR v) noexcept {
	x = DX::XMVectorGetX(v);
	y = DX::XMVectorGetY(v);
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

/////////////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////////////


FORCEINLINE FloatStore<4>::FloatStore(DX::FXMVECTOR v) noexcept {
	x = DX::XMVectorGetX(v);
	y = DX::XMVectorGetY(v);
	z = DX::XMVectorGetZ(v);
	w = DX::XMVectorGetW(v);
}

inline FloatStore<4>::FloatStore(const FloatStore<2> &f2, float z, float w) noexcept : FloatStore(f2.x, f2.y, z, w) {
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

template <typename T, bool EnableAssign, size_t... I>
template <typename> requires (EnableAssign)
T &Swizzle<T, EnableAssign, I...>::operator=(const T &other) noexcept {
	((at<I>() = other[I]), ...);
	return *this;
}

template <typename T, bool EnableAssign, size_t... I>
Swizzle<T, EnableAssign, I...>::operator T() const noexcept {
	return T(at<I>()...);
}

template<typename T, bool EnableAssign, size_t... I>
FORCEINLINE std::ostream &operator<<(std::ostream &os, const Swizzle<T, EnableAssign, I...> &sz) noexcept {
	os << static_cast<T>(sz);
	return os;
}

template <typename T, bool EnableAssign, size_t... I>
template <size_t Idx>
float &Swizzle<T, EnableAssign, I...>::at() noexcept {
	return reinterpret_cast<float *>(this)[Idx];
}

template <typename T, bool EnableAssign, size_t... I>
template <size_t Idx>
float Swizzle<T, EnableAssign, I...>::at() const noexcept {
	return reinterpret_cast<const float *>(this)[Idx];
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////////////////////////////


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


////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////
FORCEINLINE BoolVector::BoolVector(DX::FXMVECTOR vec) noexcept : _vec(vec) {
}
FORCEINLINE BoolVector::operator DX::XMVECTOR() const {
	return _vec;
}
///////////////////////////////////////////////////////////////////////////////////////
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

FORCEINLINE Vector4::Vector4(const float3 &f3, float v) noexcept : Vector4(f3.x, f3.y, f3.z, v) {
};
FORCEINLINE Vector4::Vector4(const float4 &f4) noexcept : Vector4(f4.x, f4.y, f4.z, f4.w) {
};
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

}
