#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include "UploadBuffer.h"
#include "Math/VectorHelper.h"
#include "Math/MatrixHelper.h"
#include "D3D/Material.h"

namespace d3dUlti {

struct Material;
struct MaterialConstants;

using namespace mat;
using namespace vec;

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;

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
	d3dUlti::MaterialConstants gLights[kMaxLights];
};

struct ObjectConstants {
	float4x4	gWorld;
};

class FrameResource {
public:
	FrameResource(ID3D12Device *device, UINT passCount, UINT objectCount);
	~FrameResource() = default;
public:
	WRL::ComPtr<ID3D12CommandAllocator>	cmdListAlloc_ = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB_ = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB_ = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>> materialCB_ = nullptr;
	UINT fence_ = 0;
};

};
