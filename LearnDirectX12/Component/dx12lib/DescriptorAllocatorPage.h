#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
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
	uint32 computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;
	void addNewBlock(std::size_t offset, std::size_t numDescriptor);
	void freeBlock(std::size_t offset, std::size_t numDescriptor);
private:
	friend class Device;
	struct FreeBlockInfo;
	using OffsetType = std::size_t;
	using SizeType = std::size_t;
	using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;
	using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;
	struct FreeBlockInfo {
		std::size_t  size;
		FreeListBySize::iterator sizeIter;
	};

	struct StaleDescriptorInfo {
		uint32 offset;
		uint32 size;
	};
private:
	uint32                            _numFreeHandle;
	uint32                            _numDescriptorInHeap;
	uint32                            _descriptorHandleIncrementSize;
	CD3DX12_CPU_DESCRIPTOR_HANDLE     _baseDescriptor;
	mutable std::mutex                _allocationMutex;
	std::weak_ptr<Device>             _pDevice;
	D3D12_DESCRIPTOR_HEAP_TYPE        _heapType;
	std::queue<StaleDescriptorInfo>   _staleAllocation;
	WRL::ComPtr<ID3D12DescriptorHeap> _pDescriptorHeap;
	FreeListBySize                    _freeListBySize;
	FreeListByOffset                  _freeListByOffset;

};

}