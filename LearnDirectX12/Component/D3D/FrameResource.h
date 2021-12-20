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
	float4x4		gView;
	float4x4		gInvView;
	float4x4		gProj;
	float4x4		gInvProj;
	float4x4		gViewProj;
	float4x4		gInvViewProj;
	float3			gEyePos;
	float			cbPerObjectPad1;
	float2			gRenderTargetSize;
	float2			gInvRenderTargetSize;
	float			gNearZ;
	float			gFarZ;
	float			gTotalTime;
	float			gDeltaTime;
	float4			gAmbientLight;
	d3dUtil::Light	gLights[kMaxLights];
};

struct ObjectConstants {
	float4x4	gWorldMat;
	float4x4	gNormalMat;
	float4x4	gTexMat;
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
