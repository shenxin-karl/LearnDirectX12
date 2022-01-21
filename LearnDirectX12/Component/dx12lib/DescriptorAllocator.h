#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class DescriptorAllocatorPage;
class DescriptorAllocation;
class DescriptorAllocator {
public:
	DescriptorAllocation allocate(uint32 numDescriptor = 1);
	void releaseStateDescriptors();
	uint32 getNumDescriptorPerHeap() const noexcept;
	void setNumDescriptorPerHeap(uint32 num) noexcept;
	D3D12_DESCRIPTOR_HEAP_TYPE getHeapType() const noexcept;
//private:
	friend class Device;
	using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;
	std::shared_ptr<DescriptorAllocatorPage> createAllocatorPage();
	DescriptorAllocator(std::weak_ptr<Device> pDevice, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 numDescriptorPreHeap);
private:
	uint32					    _numDescriptorPreHeap;
	std::weak_ptr<Device>       _pDevice;
	D3D12_DESCRIPTOR_HEAP_TYPE  _heapType;
	std::mutex				    _allocationMutex;
	DescriptorHeapPool		    _heapPool;
	std::unordered_set<size_t>  _availableHeaps;
};

}