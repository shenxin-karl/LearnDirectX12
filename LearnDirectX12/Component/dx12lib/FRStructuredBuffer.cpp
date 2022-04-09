#include "FRStructuredBuffer.hpp"
#include "Device.h"

namespace dx12lib {

FRStructuredBuffer<RawData>::FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void* pData) {
	auto pSharedDevice = pDevice.lock();
	_structedBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		kFrameResourceCount,
		sizeInByte,
		false
	);

	_structedBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		if (pData != nullptr)
			_pUploadBuffer->copyData(i, pData, sizeInByte, 0);

		
	}
}
}
