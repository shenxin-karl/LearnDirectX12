#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include <ostream>

namespace DX = DirectX;
#define FORCEINLINE __forceinline

namespace Math {

using size_t = std::size_t;
template<typename T, size_t N>
struct VectorHelper;

using float2 = VectorHelper<float, 2>;
using float3 = VectorHelper<float, 3>;
using float4 = VectorHelper<float, 4>;

template<>
struct VectorHelper<float, 2> : public DX::XMFLOAT2 {
public:
	FORCEINLINE VectorHelper() = default;
	FORCEINLINE VectorHelper(const VectorHelper &) = default;
	FORCEINLINE VectorHelper(VectorHelper &&) = default;
	FORCEINLINE VectorHelper &operator=(const VectorHelper &) = default;
	FORCEINLINE VectorHelper &operator=(VectorHelper &&) = default;
	using DX::XMFLOAT2::XMFLOAT2;
	using DX::XMFLOAT2::operator=;
	FORCEINLINE explicit VectorHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit VectorHelper(T v) 
	: DX::XMFLOAT2(static_cast<float>(v), static_cast<float>(v)) {
	
	}
	template<typename T1, typename T2> 
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float>)
	FORCEINLINE VectorHelper(T1 x, T2 y)
	: DX::XMFLOAT2(float(x), float(y)) {

	}
	FORCEINLINE DX::XMVECTOR toVec() const {
		DX::XMVECTOR v = DX::XMLoadFloat2(this);
		return v;
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const {
		return toVec();
	}
	FORCEINLINE float &operator[](size_t n) {
		assert(n < 2);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const {
		assert(n < 2);
		return reinterpret_cast<const float *>(this)[n];
	}
};

VectorHelper<float, 3> cross(const VectorHelper<float, 3> &lhs, const VectorHelper<float, 3> &rhs);

template<>
struct VectorHelper<float, 3> : public DX::XMFLOAT3 {
public:
	FORCEINLINE VectorHelper() = default;
	FORCEINLINE VectorHelper(const VectorHelper &) = default;
	FORCEINLINE VectorHelper(VectorHelper &&) = default;
	FORCEINLINE VectorHelper &operator=(const VectorHelper &) = default;
	FORCEINLINE VectorHelper &operator=(VectorHelper &&) = default;
	using DX::XMFLOAT3::XMFLOAT3;
	using DX::XMFLOAT3::operator=;
	FORCEINLINE explicit VectorHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit VectorHelper(T v) 
	: XMFLOAT3(float(v), float(v), float(v)) {

	}
	template<typename T1, typename T2, typename T3>
	requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
			 std::is_convertible_v<T3, float>)
	FORCEINLINE VectorHelper(T1 x, T2 y, T3 z)
	: DX::XMFLOAT3(float(x), float(y), float(z)) {

	}
	FORCEINLINE DX::XMVECTOR toVec() const {
		DX::XMVECTOR v = DX::XMLoadFloat3(this);
		return v;
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const {
		return toVec();
	}
	FORCEINLINE float &operator[](size_t n) {
		assert(n < 3);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const {
		assert(n < 3);
		return reinterpret_cast<const float *>(this)[n];
	}
	FORCEINLINE friend VectorHelper cross(const VectorHelper &lhs, const VectorHelper &rhs) {
		return {
			(lhs.y*rhs.z - lhs.z*rhs.y),
			(lhs.z*rhs.x - lhs.x*rhs.z),
			(lhs.x*rhs.y - lhs.y*rhs.x), 
		};
	}
	FORCEINLINE explicit VectorHelper(const DX::XMVECTORF32 &color) 
		: VectorHelper(color.operator DirectX::XMVECTOR()){
	}
};

template<>
struct VectorHelper<float, 4> : public DX::XMFLOAT4 {
public:
	FORCEINLINE VectorHelper() = default;
	FORCEINLINE VectorHelper(const VectorHelper &) = default;
	FORCEINLINE VectorHelper(VectorHelper &&) = default;
	FORCEINLINE VectorHelper &operator=(const VectorHelper &) = default;
	FORCEINLINE VectorHelper &operator=(VectorHelper &&) = default;
	using DX::XMFLOAT4::XMFLOAT4;
	using DX::XMFLOAT4::operator=;
	FORCEINLINE explicit VectorHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
		w = DX::XMVectorGetW(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit VectorHelper(T v) 
	: DX::XMFLOAT4(float(v), float(v), float(v), float(v)) {

	}
	template<typename T1, typename T2, typename T3, typename T4>
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float> &&
			 std::is_convertible_v<T3, float> && std::is_convertible_v<T4, float>)
	FORCEINLINE VectorHelper(T1 x, T2 y, T3 z, T4 w)
	: DX::XMFLOAT4(float(x), float(y), float(z), float(w)) {

	}
	FORCEINLINE DX::XMVECTOR toVec() const {
		DX::XMVECTOR v = DX::XMLoadFloat4(this);
		return v;
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const {
		return toVec();
	}
	FORCEINLINE float &operator[](size_t n) {
		assert(n < 4);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const {
		assert(n < 4);
		return reinterpret_cast<const float *>(this)[n];
	}
	FORCEINLINE explicit VectorHelper(const DX::XMVECTORF32 &color)
		: VectorHelper(color.operator DirectX::XMVECTOR())
	{}
};

template<typename T, size_t N>
FORCEINLINE T dot(const VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);

template<typename T, size_t N>
FORCEINLINE T length(const VectorHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE T lengthSqr(const VectorHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> normalize(const VectorHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE std::ostream &operator<<(std::ostream &os, const VectorHelper<T, N> &vec);

/*----------------------------- add --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator+(const VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator+(T1 lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE VectorHelper<T, N> &operator+=(VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  VectorHelper<T, N> operator+(const VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> &operator+=(VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
/*----------------------------- add --------------------------------*/
/*----------------------------- sub --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator-(const VectorHelper<T, N> &lhs, T rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator-(T1 lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE VectorHelper<T, N> &operator-=(VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  VectorHelper<T, N> operator-(const VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> &operator-=(VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
/*----------------------------- sub --------------------------------*/
/*----------------------------- mul --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator*(const VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator*(T1 lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE VectorHelper<T, N> &operator*=(VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  VectorHelper<T, N> operator*(const VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> &operator*=(VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
/*----------------------------- mul --------------------------------*/
/*----------------------------- div --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator/(const VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  VectorHelper<T, N> operator/(T1 lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE VectorHelper<T, N> &operator/=(VectorHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  VectorHelper<T, N> operator/(const VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> &operator/=(VectorHelper<T, N> &lhs, const VectorHelper<T, N> &rhs);
/*----------------------------- div --------------------------------*/
template<typename T, size_t N>
FORCEINLINE VectorHelper<T, N> operator-(const VectorHelper<T, N> &v);

}

#include "VectorHelper.ini"