#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Math/MathHelper.h"

namespace d3dUtil {

using namespace Math;
namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

struct PassConstants {
	float4x4 gView = MathHelper::identity4x4();
	float4x4 gInvView = MathHelper::identity4x4();
	float4x4 gProj = MathHelper::identity4x4();
	float4x4 gInvProj = MathHelper::identity4x4();
	float4x4 gViewProj = MathHelper::identity4x4();
	float4x4 gInvViewProj = MathHelper::identity4x4();
	float3	 gEyePos = float3(0.f);
	float	 cbPerObjectPad1 = 0.f;
	float2	 gRenderTargetSize = float2(0.f);
	float2	 gInvRenderTargetSize = float2(0.f);
	float	 gNearZ = 0.f;
	float	 gFarZ = 0.f;
	float	 gTotalTime = 0.f;
	float	 gDeltaTime = 0.f;
};

};
