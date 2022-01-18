#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class DescriptorAllocatorPage;

class DescriptorAllocation {
	DescriptorAllocation();
	DescriptorAllocation(const DescriptorAllocation &) = delete;
	DescriptorAllocation(DescriptorAllocation &&other) noexcept;
	DescriptorAllocation &operator=(const DescriptorAllocation &) = delete;
	DescriptorAllocation &operator=(DescriptorAllocation &&other) noexcept;
	~DescriptorAllocation();
	friend void swap(DescriptorAllocation &lhs, DescriptorAllocation &rhs) noexcept;
	DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE handle,
		uint32 numHandle,
		uint32 handleSize,
		std::shared_ptr<DescriptorAllocatorPage> pPage
	);
public:
	uint32 getNumHandle() const noexcept;
	uint32 getHandleSize() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE getBaseHandle() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE getHandleByOffset(uint32 offset) const;
	bool isNull() const noexcept;
	bool isValid() const noexcept;
	void free();
private:
	friend class DescriptorAllocatorPage;
	void clear() noexcept;
private:
	uint32 _numHandle;						 // 已经分配的描述符数量
	uint32 _handleSize;						 // 单个描述符的大小
	D3D12_CPU_DESCRIPTOR_HANDLE _baseHandle; // 描述符的首个 handle
	std::shared_ptr<DescriptorAllocatorPage> _pPage; // 描述符所在的描述符页
};

}