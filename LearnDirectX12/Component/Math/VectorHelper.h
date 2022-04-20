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
	requires(std::is_convertible_v<T1, float> && std::is_convertible_v<T2, float>)
	FORCEINLINE FloatStore(T1 x, T2 y) noexcept;
};

template<>
struct FloatStore<3> : public DX::XMFLOAT3 {
	using DX::XMFLOAT3::XMFLOAT3;
	using DX::XMFLOAT3::operator=;
	FORCEINLINE FloatStore() noexcept = default;
	FORCEINLINE FloatStore(const FloatStore &) noexcept = default;
	FORCEINLINE FloatStore(FloatStore &&) noexcept = default;
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
	FORCEINLINE operator FloatStore<N> &() noexcept;

	template<size_t N> requires(N <= 3)
	FORCEINLINE operator const FloatStore<N> &() const noexcept;
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
	FORCEINLINE operator FloatStore<N> &() noexcept;

	template<size_t N> requires(N <= 4)
	FORCEINLINE operator const FloatStore<N> &() const noexcept;
};

using float2 = FloatStore<2>;
using float3 = FloatStore<3>;
using float4 = FloatStore<4>;

#include "FloatStore.ini"

}