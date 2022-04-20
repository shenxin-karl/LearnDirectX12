#pragma once
#include <ostream>
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

namespace Math {

template<typename T, bool EnableAssign, size_t...I>
struct Swizzle {
	Swizzle() noexcept = default;
	template<typename = void> requires(EnableAssign)
	FORCEINLINE T &operator=(const T &other) noexcept {
		((at<I>() = other[I]), ...);
		return *this;
	}
	FORCEINLINE operator T() const noexcept {
		return T(at<I>()...);
	}
	FORCEINLINE friend std::ostream &operator<<(std::ostream &os, const Swizzle &sz) noexcept {
		os << static_cast<T>(sz);
		return os;
	}
public:
	template<size_t Idx>
	FORCEINLINE float &at() noexcept {
		return reinterpret_cast<float *>(this)[Idx];
	}
	template<size_t Idx>
	FORCEINLINE float at() const noexcept {
		return reinterpret_cast<const float *>(this)[Idx];
	}
};

}
