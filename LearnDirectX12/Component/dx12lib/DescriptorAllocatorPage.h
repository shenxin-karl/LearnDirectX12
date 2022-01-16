#pragma once
#include "dx12libCommon.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class DescriptorAllocatorPage {
public:
	DescriptorAllocatorPage();
	DescriptorAllocatorPage(const DescriptorAllocatorPage &) = delete;
	DescriptorAllocatorPage(DescriptorAllocatorPage &&other) noexcept;
	DescriptorAllocatorPage &operator=(const DescriptorAllocatorPage &) = delete;
	DescriptorAllocatorPage &operator=(DescriptorAllocatorPage &&other) noexcept;
	friend void swap(DescriptorAllocatorPage &lhs, DescriptorAllocatorPage &rhs) noexcept;

	DescriptorAllocatorPage(std::weak_ptr<Device> pDevice, 
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
		uint32 numDescriptorPreHeap
	);
public:
	D3D12_DESCRIPTOR_HEAP_TYPE getHeapType() const;
	DescriptorAllocation allocate(uint32 numDescriptor);
	uint32 getFreeHandle() const;
	bool hasSpace(uint32 numDescriptor) const;
	void free(DescriptorAllocation &&allocation);
	void releaseStaleDescriptors();
private:
	uint32                            _numFreeHandle;
	uint32                            _numDescriptorInHeap;
	uint32                            _descriptorHandleIncrementSize;
	mutable std::mutex                _allocationMutex;
	std::weak_ptr<Device>             _pDevice;
	D3D12_DESCRIPTOR_HEAP_TYPE        _heapType;
	std::vector<DescriptorAllocation> _staleAllocation;
	WRL::ComPtr<ID3D12DescriptorHeap> _pDescriptorHeap;
};

}