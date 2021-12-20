#pragma once
#include <string>
#include "Math/MathHelper.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"

namespace d3dUtil {

using namespace vec;
using namespace mat;

extern inline int kNumFrameResources;

struct MaterialConstants {
	float4   gDiffuseAlbedo  = float4(1.f);
	float3   gFresnelR0		= float3(0.1f);
	float	 gMatPad1		= 0.f;
	float    gRoughness		= 0.25f;
	float	 gMetallic		= 0.f;
	float2   gMatPad2		= float2(0.f);
	float4x4 gMatTransfrom	= MathHelper::identity4x4();
};

struct Material {
	std::string	name_;
	int			matCBIndex_ = -1;
	int			diffuseSrvHeapIndex_ = -1;
	int			numFrameDirty_ = kNumFrameResources;
	float4		diffuseAlbedo_ = float4(1.f);
	float3		fresnelR0 = float3(0.1f);
	float		roughness_ = 0.25f;
	float		metallic_  = 0.5f;
	float4x4	matTransfrom_ = MathHelper::identity4x4();
};

};