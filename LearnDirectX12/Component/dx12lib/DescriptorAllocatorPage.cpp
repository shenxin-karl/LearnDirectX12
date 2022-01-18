#include "DescriptorAllocatorPage.h"
#include "Device.h"

namespace dx12lib {

DescriptorAllocatorPage::DescriptorAllocatorPage() 
: _numFreeHandle(0), _numDescriptorInHeap(0), _descriptorHandleIncrementSize(0)
, _heapType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES), _baseDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE(0)) {
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

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = numDescriptorPreHeap;
	heapDesc.Type = _heapType;
	ThrowIfFailed(pD3DDevice->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&_pDescriptorHeap)
	));

	_numFreeHandle = numDescriptorPreHeap;
	_numDescriptorInHeap = numDescriptorPreHeap;
	_descriptorHandleIncrementSize = pD3DDevice->GetDescriptorHandleIncrementSize(_heapType);
	_baseDescriptor = _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	addNewBlock(0, numDescriptorPreHeap);
}

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::getHeapType() const {
	return _heapType;
}

uint32 DescriptorAllocatorPage::getFreeHandle() const {
	return _numFreeHandle;
}

DescriptorAllocation DescriptorAllocatorPage::allocate(uint32 numDescriptor) {
	std::lock_guard lock(_allocationMutex);
	if (numDescriptor > _numFreeHandle)
		return {};

	// A block of the appropriate size was found 
	auto iter = _freeListBySize.lower_bound(numDescriptor);
	if (iter == _freeListBySize.end())
		return {};

	uint32 size = static_cast<uint32>(iter->first);
	uint32 offset = static_cast<uint32>(iter->second->first);
	auto descriptor = _baseDescriptor;
	descriptor.Offset(offset, _descriptorHandleIncrementSize);
	DescriptorAllocation ret = {
		static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(descriptor),
		numDescriptor,
		_descriptorHandleIncrementSize,
		this->shared_from_this(),
	};
	
	// 拆开内存块
	_freeListByOffset.erase(iter->second);
	_freeListBySize.erase(iter);
	uint32 newSize = size - numDescriptor;
	uint32 newOffset = offset + numDescriptor;
	_numFreeHandle -= numDescriptor;
	if (newSize > 0)
		addNewBlock(newOffset, newSize);

	return ret;
}


void DescriptorAllocatorPage::free(DescriptorAllocation &&allocation) {
	auto temp = std::move(allocation);
	StaleDescriptorInfo staleInfo = {
		computeOffset(allocation.getBaseHandle()),
		allocation.getHandleSize(),
	};
	temp.clear();
	std::lock_guard lock(_allocationMutex);
	_staleAllocation.push(staleInfo);
}

void DescriptorAllocatorPage::releaseStaleDescriptors() {
	while (!_staleAllocation.empty()) {
		auto staleInfo = _staleAllocation.front();
		_staleAllocation.pop();
		freeBlock(staleInfo.offset, staleInfo.size);
	}
}

uint32 DescriptorAllocatorPage::computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const {
	auto offset = (handle.ptr - _baseDescriptor.ptr) / _descriptorHandleIncrementSize;
	return static_cast<uint32>(offset);
}

void DescriptorAllocatorPage::addNewBlock(std::size_t offset, std::size_t numDescriptor) {
	auto offsetIt = _freeListByOffset.emplace(offset, FreeBlockInfo{});
	auto sizeIt = _freeListBySize.emplace(numDescriptor, offsetIt.first);
	offsetIt.first->second.sizeIter = sizeIt;
}

void DescriptorAllocatorPage::freeBlock(std::size_t offset, std::size_t numDescriptor) {
	auto nextOffsetIt = _freeListByOffset.lower_bound(offset);
	auto newOffset = offset;
	auto newSize = numDescriptor;

	//        prev             current               next
	// |----------------|------------------|--------------------|

	// try merge previous block
	if (nextOffsetIt != _freeListByOffset.begin()) {
		auto prevOffsetIt = nextOffsetIt;
		--prevOffsetIt;
		if (prevOffsetIt->first + prevOffsetIt->second.size == offset) {	// Is a continuous block  
			newOffset -= prevOffsetIt->first;
			newSize += prevOffsetIt->second.size;
			_freeListBySize.erase(prevOffsetIt->second.sizeIter);
			_freeListByOffset.erase(prevOffsetIt);
		}
	}

	// try merge next block
	if (nextOffsetIt != _freeListByOffset.end()) {
		if ((offset + numDescriptor == nextOffsetIt->first))				// Is a continuous block  
			newSize += nextOffsetIt->second.size;
		_freeListBySize.erase(nextOffsetIt->second.sizeIter);
		_freeListByOffset.erase(nextOffsetIt);
	}

	addNewBlock(newOffset, newSize);
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