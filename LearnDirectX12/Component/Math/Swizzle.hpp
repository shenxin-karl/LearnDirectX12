#pragma once

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

namespace Math {

template<typename T, bool EnableAssign, size_t...I>
struct Swizzle {
	FORCEINLINE T &operator=(const T &other) {
		((at<I>() = other[I]), ...);
		return *this;
	}
	FORCEINLINE operator T() const noexcept {
		return T(at<I>()...);
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
