#include "FrameResource.h"

namespace d3dUtil {

FrameResource::FrameResource(ID3D12Device *device, const FrameResourceDesc &desc) {
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdListAlloc_)
	));

	if (desc.passCount > 0)
		passCB_ = std::make_unique<UploadBuffer<PassConstants>>(device, desc.passCount, true);
	if (desc.objectCount > 0)
		objectCB_ = std::make_unique<UploadBuffer<ObjectConstants>>(device, desc.objectCount, true);
	if (desc.materialCount > 0)
		materialCB_ = std::make_unique<UploadBuffer<MaterialConstants>>(device, desc.materialCount, true);
}

}