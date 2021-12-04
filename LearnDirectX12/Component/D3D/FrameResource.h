#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>
#include "UploadBuffer.h"

namespace DX = DirectX;
namespace WRL = Microsoft::WRL;
struct PassConstants {
	DX::XMFLOAT4X4	gView;
	DX::XMFLOAT4X4	gInvView;
	DX::XMFLOAT4X4  gProj;
	DX::XMFLOAT4X4	gInvProj;
	DX::XMFLOAT4X4	gViewProj;
	DX::XMFLOAT4X4	gInvViewProj;
	DX::XMFLOAT3	gEyePos;
	float			cbPerObjectPad1;
	DX::XMFLOAT2	gRenderTargetSize;
	DX::XMFLOAT2	gInvRenderTargetSize;
	float			gNearZ;
	float			gFarZ;
	float			gTotalTime;
	float			gDeltaTime;
};

struct ObjectConstants {
	DX::XMFLOAT4X4	gWorld;
};

class FrameResource {
public:
	FrameResource(ID3D12Device *device, UINT passCount, UINT objectCount);
	~FrameResource() = default;
public:
	WRL::ComPtr<ID3D12CommandAllocator>	cmdListAlloc_ = nullptr;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB_ = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB_ = nullptr;
	UINT fence_ = 0;
};

