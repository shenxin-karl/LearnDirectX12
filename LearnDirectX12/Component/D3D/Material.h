#pragma once
#include <string>
#include "Math/MathHelper.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"

namespace d3dUlti {

using namespace vec;
using namespace mat;

extern inline int kNumFrameResources;

struct MaterialConstants {
	float4   diffuseAlbedo_ = float4(1.f);
	float3   fresnelR0		= float3(0.1f);
	float    roughness		= 0.25f;
	float4x4 matTransfrom	= MathHelper::identity4x4();
};

struct Material {
	std::string	name_;
	int			matCBIndex_ = -1;
	int			diffuseSrvHeapIndex_ = -1;
	int			numFrameDirty = kNumFrameResources;
	float4		diffuseAlbedo_ = float4(1.f);
	float3		freshnelR0 = float3(0.1f);
	float		roughness_ = 0.25f;
	float4x4	matTransfrom = MathHelper::identity4x4();
};

};