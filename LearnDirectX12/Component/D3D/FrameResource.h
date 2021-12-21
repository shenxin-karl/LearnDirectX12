#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include "UploadBuffer.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"
#include "D3D/Material.h"
#include "D3D/Light.h"

namespace d3dUtil {

struct Material;
struct MaterialConstants;

enum CBRegisterType : UINT {
	CB_Object	= 0,
	CB_Pass		= 1,
	CB_Material = 2,
};

using namespace mat;
using namespace vec;

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

constexpr inline int kMaxLights = 16;

struct PassConstants {
	float4x4		gView = MathHelper::identity4x4();
	float4x4		gInvView = MathHelper::identity4x4();
	float4x4		gProj = MathHelper::identity4x4();
	float4x4		gInvProj = MathHelper::identity4x4();
	float4x4		gViewProj = MathHelper::identity4x4();
	float4x4		gInvViewProj = MathHelper::identity4x4();
	float3			gEyePos = float3(0.f);
	float			cbPerObjectPad1 = 0.f;
	float2			gRenderTargetSize = float2(0.f);
	float2			gInvRenderTargetSize = float2(0.f);
	float			gNearZ = 0.f;
	float			gFarZ = 0.f;
	float			gTotalTime = 0.f;
	float			gDeltaTime = 0.f;
	float4			gAmbientLight = float4(0.f);
	d3dUtil::Light	gLights[kMaxLights];
};

struct ObjectConstants {
	float4x4	gWorldMat;
	float4x4	gNormalMat;
	float4x4	gTextureMat;
};

struct FrameResourceDesc {
	UINT passCount		= 1;
	UINT objectCount	= 0;
	UINT materialCount	= 0;
};

class FrameResource {
public:
	FrameResource(ID3D12Device *device, const FrameResourceDesc &desc);
	~FrameResource() = default;
	WRL::ComPtr<ID3D12CommandAllocator>	cmdListAlloc_ = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB_ = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB_ = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> materialCB_ = nullptr;
	UINT fence_ = 0;
};

};
