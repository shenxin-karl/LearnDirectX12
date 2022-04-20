#pragma once
#include <DirectXMath.h>
#include "MathHelper.h"

namespace Math {

namespace DX = DirectX;

class alignas(sizeof(DX::XMVECTOR)) Scalar {
public:
    FORCEINLINE Scalar() = default;
    FORCEINLINE Scalar(const Scalar &) = default;
    FORCEINLINE Scalar &operator=(const Scalar &) = default;

    FORCEINLINE Scalar(DX::XMVECTOR vec) noexcept : vec(vec) {
    }
    FORCEINLINE explicit Scalar(float v) noexcept : vec(DX::XMVectorSet(v, v, v, v)) {
	}
    FORCEINLINE operator float () const noexcept {
		return  DX::XMVectorGetX(vec);
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
    FORCEINLINE friend Scalar operator- (Scalar s) noexcept {
	    return Scalar(DX::XMVectorNegate(s));
    }
    FORCEINLINE friend Scalar operator+ (Scalar s1, Scalar s2) noexcept {
	    return Scalar(DX::XMVectorAdd(s1, s2));
    }
    FORCEINLINE friend Scalar operator- (Scalar s1, Scalar s2) noexcept {
	    return Scalar(DX::XMVectorSubtract(s1, s2));
    }
    FORCEINLINE friend Scalar operator* (Scalar s1, Scalar s2) noexcept {
	    return Scalar(DX::XMVectorMultiply(s1, s2));
    }
    FORCEINLINE friend Scalar operator/ (Scalar s1, Scalar s2) noexcept {
	    return Scalar(DX::XMVectorDivide(s1, s2));
    }
    FORCEINLINE friend Scalar operator+ (Scalar s1, float s2) noexcept {
	    return s1 + Scalar(s2);
    }
    FORCEINLINE friend Scalar operator- (Scalar s1, float s2) noexcept {
	    return s1 - Scalar(s2);
    }
    FORCEINLINE friend Scalar operator* (Scalar s1, float s2) noexcept {
	    return s1 * Scalar(s2);
    }
    FORCEINLINE friend Scalar operator/ (Scalar s1, float s2) noexcept {
	    return s1 / Scalar(s2);
    }
    FORCEINLINE friend Scalar operator+ (float s1, Scalar s2) noexcept {
	    return Scalar(s1) + s2;
    }
    FORCEINLINE friend Scalar operator- (float s1, Scalar s2) noexcept {
	    return Scalar(s1) - s2;
    }
    FORCEINLINE friend Scalar operator* (float s1, Scalar s2) noexcept {
	    return Scalar(s1) * s2;
    }
    FORCEINLINE friend Scalar operator/ (float s1, Scalar s2) noexcept {
	    return Scalar(s1) / s2;
    }
private:
	DX::XMVECTOR vec;
};


}
