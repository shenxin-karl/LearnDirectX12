#pragma once
#include <DirectXMath.h>


namespace Math {

namespace DX = DirectX;

template<typename T, size_t C, size_t R>
struct MatrixHelper;

using float4x4 = DX::XMFLOAT4X4;
using float4x3 = DX::XMFLOAT4X3;
using float3x3 = DX::XMFLOAT3X3;
using float3x4 = DX::XMFLOAT3X4;

template<>
struct MatrixHelper<float, 4, 4> : public DX::XMFLOAT4X4 {
	using DX::XMFLOAT4X4::XMFLOAT4X4;
};

}

