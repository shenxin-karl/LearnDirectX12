#pragma once
#include <DirectXMath.h>
#include <type_traits>
#include <ostream>

namespace DX = DirectX;
#define FORCEINLINE __forceinline

namespace Math {

template<size_t N>
struct FloatStore;

template<>
struct FloatStore<2> : public DX::XMFLOAT2 {
	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	using DX::XMFLOAT2::XMFLOAT2;
	using DX::XMFLOAT2::operator=;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v) noexcept : DX::XMFLOAT2(static_cast<float>(v), static_cast<float>(v)) {
	}
	template<typename T1, typename T2>
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float>)
	FORCEINLINE FloatStore(T1 x, T2 y) noexcept : DX::XMFLOAT2(float(x), float(y)) {
	}
	FORCEINLINE float &operator[](size_t n) noexcept {
		assert(n < 2);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const noexcept {
		assert(n < 2);
		return reinterpret_cast<const float *>(this)[n];
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const FloatStore &v) noexcept {
		os << '(' << v.x << ", " << v.y << ')';
		return os;
	}
	FORCEINLINE FloatStore operator-() const noexcept {
		return FloatStore(-x, -y);
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept {
		return DX::XMVectorSet(x, y, 0.f, 0.f);
	}
};


template<>
struct FloatStore<3> : public DX::XMFLOAT3 {
	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	using DX::XMFLOAT3::XMFLOAT3;
	using DX::XMFLOAT3::operator=;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v)
	: XMFLOAT3(float(v), float(v), float(v)) {

	}
	template<typename T1, typename T2, typename T3>
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float> &&
	std::is_convertible_v<T3, float>)
	FORCEINLINE FloatStore(T1 x, T2 y, T3 z) noexcept
	: DX::XMFLOAT3(float(x), float(y), float(z)) {

	}
	template<size_t N> requires(N <= 3)
	FORCEINLINE operator FloatStore<N> &() noexcept {
		return reinterpret_cast<FloatStore<N> &>(*this);
	}
	template<size_t N> requires(N <= 3)
	FORCEINLINE operator const FloatStore<N> &() const noexcept {
		return reinterpret_cast<const FloatStore<N> &>(*this);
	}
	FORCEINLINE float &operator[](size_t n) noexcept {
		assert(n < 3);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const noexcept {
		assert(n < 3);
		return reinterpret_cast<const float *>(this)[n];
	}
	FORCEINLINE explicit FloatStore(const DX::XMVECTORF32 & color) noexcept
	: FloatStore(color.operator DirectX::XMVECTOR()) {
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const FloatStore &v) noexcept {
		os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os;
	}
	FloatStore operator-() const noexcept {
		return FloatStore(-x, -y, -z);
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept {
		return DX::XMVectorSet(x, y, z, 0.f);
	}
};

template<>
struct FloatStore<4> : public DX::XMFLOAT4 {
	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
	FORCEINLINE FloatStore &operator=(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore &operator=(FloatStore &&) noexcept = default;
	using DX::XMFLOAT4::XMFLOAT4;
	using DX::XMFLOAT4::operator=;
	FORCEINLINE explicit FloatStore(DX::FXMVECTOR v) noexcept {
		x = DX::XMVectorGetX(v);
		y = DX::XMVectorGetY(v);
		z = DX::XMVectorGetZ(v);
		w = DX::XMVectorGetW(v);
	}
	template<typename T> requires(std::is_convertible_v<T, float>)
	FORCEINLINE explicit FloatStore(T v) noexcept : DX::XMFLOAT3(float(v), float(v), float(v), float(v)) {
	}
	template<typename T1, typename T2, typename T3, typename T4>
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float> && 
			 std::is_convertible_v<T3, float> && std::is_convertible_v<T4, float>)
	FORCEINLINE FloatStore(T1 x, T2 y, T3 z, T4 w) noexcept : DX::XMFLOAT3(float(x), float(y), float(z), float(w)) {
	}
	template<size_t N> requires(N <= 4)
	FORCEINLINE operator FloatStore<N> &() noexcept {
		return reinterpret_cast<FloatStore<N> &>(*this);
	}
	template<size_t N> requires(N <= 4)
		FORCEINLINE operator const FloatStore<N> &() const noexcept {
		return reinterpret_cast<const FloatStore<N> &>(*this);
	}
	FORCEINLINE float &operator[](size_t n) noexcept {
		assert(n < 4);
		return reinterpret_cast<float *>(this)[n];
	}
	FORCEINLINE float operator[](size_t n) const noexcept {
		assert(n < 4);
		return reinterpret_cast<const float *>(this)[n];
	}
	FORCEINLINE explicit FloatStore(const DX::XMVECTORF32 & color) noexcept
	: FloatStore(color.operator DirectX::XMVECTOR()) {
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const FloatStore &v) noexcept {
		os << '(' << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ')';
		return os;
	}
	FORCEINLINE FloatStore operator-() const noexcept {
		return FloatStore(-x, -y, -z, -w);
	}
	FORCEINLINE explicit operator DX::XMVECTOR() const noexcept {
		return DX::XMLoadFloat4(this);
	}
};

using float2 = FloatStore<2>;
using float3 = FloatStore<3>;
using float4 = FloatStore<4>;

}