#include "DescriptorAllocatorPage.h"
#include "Device.h"

namespace dx12lib {

DescriptorAllocatorPage::DescriptorAllocatorPage() 
: _numFreeHandle(0), _numDescriptorInHeap(0), _descriptorHandleIncrementSize(0)
, _heapType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES) {
}

DescriptorAllocatorPage::DescriptorAllocatorPage(DescriptorAllocatorPage &&other) noexcept 
: DescriptorAllocatorPage() 
{
	swap(*this, other);
}

DescriptorAllocatorPage::DescriptorAllocatorPage(std::weak_ptr<Device> pDevice, 
	D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	uint32 numDescriptorPreHeap) 
: _pDevice(pDevice), _heapType(heapType)
{
	ID3D12Device *pD3DDevice = pDevice.lock()->getD3DDevice();
}

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::getHeapType() const {
	return _heapType;
}

uint32 DescriptorAllocatorPage::getFreeHandle() const {
	return _numFreeHandle;
}

void DescriptorAllocatorPage::free(DescriptorAllocation &&allocation) {
	std::lock_guard lock(_allocationMutex);
	_staleAllocation.push_back(std::move(allocation));
}

DescriptorAllocatorPage &DescriptorAllocatorPage::operator=(DescriptorAllocatorPage &&other) noexcept {
	DescriptorAllocatorPage tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(DescriptorAllocatorPage &lhs, DescriptorAllocatorPage &rhs) noexcept {
	using std::swap;
	swap(lhs._numFreeHandle, rhs._numFreeHandle);
	swap(lhs._numDescriptorInHeap, rhs._numDescriptorInHeap);
	swap(lhs._descriptorHandleIncrementSize, rhs._descriptorHandleIncrementSize);
	//swap(lhs._allocationMutex, rhs._allocationMutex);
	swap(lhs._pDevice, rhs._pDevice);
	swap(lhs._heapType, rhs._heapType);
	swap(lhs._staleAllocation, rhs._staleAllocation);
	swap(lhs._pDescriptorHeap, rhs._pDescriptorHeap);
}

}