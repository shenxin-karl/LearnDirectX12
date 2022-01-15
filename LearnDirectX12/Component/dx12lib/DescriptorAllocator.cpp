#pragma once
#include "Device.h"
#include "DescriptorAllocator.h"

namespace dx12lib {

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::createAllocatorPage() {
	// todo
}

DescriptorAllocator::DescriptorAllocator(std::weak_ptr<Device> pDevice, 
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	uint32 numDescriptorPreHeap)
: _pDevice(pDevice), _numDescriptorPreHeap(numDescriptorPreHeap)
, _heapType(heapType) {
}

}
