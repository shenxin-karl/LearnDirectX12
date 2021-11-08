#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device *device, UINT passCount, UINT objectCount) {
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdListAlloc_)
	));
	
	using std::make_unique;
	passCB_ = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
	objectCB_ = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}
