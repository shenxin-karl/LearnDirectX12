#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include <ostream>

namespace DX = DirectX;
#define FORCEINLINE __forceinline

namespace vec {

template<typename T, size_t N>
struct floatHelper;

using float2 = floatHelper<float, 2>;
using float3 = floatHelper<float, 3>;
using float4 = floatHelper<float, 4>;

template<>
struct floatHelper<float, 2> : public DX::XMFLOAT2 {
public:
	FORCEINLINE floatHelper() = default;
	FORCEINLINE floatHelper(const floatHelper &) = default;
	FORCEINLINE floatHelper(floatHelper &&) = default;
	FORCEINLINE floatHelper &operator=(const floatHelper &) = default;
	FORCEINLINE floatHelper &operator=(floatHelper &&) = default;
	using DX::XMFLOAT2::XMFLOAT2;
	using DX::XMFLOAT2::operator=;
	FORCEINLINE floatHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit floatHelper(T v) 
	: DX::XMFLOAT2(static_cast<float>(v), static_cast<float>(v)) {
	
	}
	template<typename T1, typename T2> 
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float>)
	FORCEINLINE explicit floatHelper(T1 x, T2 y)
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

template<>
struct floatHelper<float, 3> : public DX::XMFLOAT3 {
public:
	floatHelper() = default;
	floatHelper(const floatHelper &) = default;
	floatHelper(floatHelper &&) = default;
	floatHelper &operator=(const floatHelper &) = default;
	floatHelper &operator=(floatHelper &&) = default;
	using DX::XMFLOAT3::XMFLOAT3;
	using DX::XMFLOAT3::operator=;
	FORCEINLINE floatHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit floatHelper(T v) 
	: XMFLOAT3(float(v), float(v), float(v)) {

	}
	template<typename T1, typename T2, typename T3>
	requires(std::is_convertible_v<T1, float> &&std::is_convertible_v<T2, float> &&
			 std::is_convertible_v<T3, float>)
	FORCEINLINE explicit floatHelper(T1 x, T2 y, T3 z)
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
	FORCEINLINE friend floatHelper cross(const floatHelper &lhs, const floatHelper &rhs) {
		return {
			-1 * (lhs.y*rhs.z - lhs.z*rhs.y),
			-1 * (lhs.z*rhs.x - lhs.x*rhs.z),
			-1 * (lhs.x*rhs.y - lhs.y*rhs.x), 
		};
	}
};

template<>
struct floatHelper<float, 4> : public DX::XMFLOAT4 {
public:
	floatHelper() = default;
	floatHelper(const floatHelper &) = default;
	floatHelper(floatHelper &&) = default;
	floatHelper &operator=(const floatHelper &) = default;
	floatHelper &operator=(floatHelper &&) = default;
	using DX::XMFLOAT4::XMFLOAT4;
	using DX::XMFLOAT4::operator=;
	FORCEINLINE floatHelper(DX::FXMVECTOR v) {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
		w = DX::XMVectorGetW(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit floatHelper(T v) 
	: DX::XMFLOAT4(float(v), float(v), float(v), float(v)) {

	}
	template<typename T1, typename T2, typename T3, typename T4>
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float> &&
			 std::is_convertible_v<T3, float> && std::is_convertible_v<T4, float>)
	FORCEINLINE explicit floatHelper(T1 x, T2 y, T3 z, T4 w)
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
};


template<typename T, size_t N>
FORCEINLINE T dot(const floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);

template<typename T, size_t N>
FORCEINLINE T length(const floatHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE T lengthSqr(const floatHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE floatHelper<T, N> normalize(const floatHelper<T, N> &vec);

template<typename T, size_t N>
FORCEINLINE std::ostream &operator<<(std::ostream &os, const floatHelper<T, N> &vec);

/*----------------------------- add --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator+(const floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator+(T1 lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE floatHelper<T, N> &operator+=(floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  floatHelper<T, N> operator+(const floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE floatHelper<T, N> &operator+=(floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
/*----------------------------- add --------------------------------*/
/*----------------------------- sub --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator-(const floatHelper<T, N> &lhs, T rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator-(T1 lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE floatHelper<T, N> &operator-=(floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  floatHelper<T, N> operator-(const floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE floatHelper<T, N> &operator-=(floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
/*----------------------------- sub --------------------------------*/
/*----------------------------- mul --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator*(const floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator*(T1 lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE floatHelper<T, N> &operator*=(floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  floatHelper<T, N> operator*(const floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE floatHelper<T, N> &operator*=(floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
/*----------------------------- mul --------------------------------*/
/*----------------------------- div --------------------------------*/
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator/(const floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE  floatHelper<T, N> operator/(T1 lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N, typename T1> requires(std::is_convertible_v<T1, T>)
FORCEINLINE floatHelper<T, N> &operator/=(floatHelper<T, N> &lhs, T1 rhs);
template<typename T, size_t N>
FORCEINLINE  floatHelper<T, N> operator/(const floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
template<typename T, size_t N>
FORCEINLINE floatHelper<T, N> &operator/=(floatHelper<T, N> &lhs, const floatHelper<T, N> &rhs);
/*----------------------------- div --------------------------------*/

}

#include "VectorHelper.ini"