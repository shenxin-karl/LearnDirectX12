#include "DescriptorAllocator.h"
#include "d3dutil.h"
#include <cassert>

namespace d3dUtil {

DescriptorAllocation::DescriptorAllocation()
: _numDescriptors(0), _descriptorSize(0), _descriptor(D3D12_CPU_DESCRIPTOR_HANDLE(0)) {
}

DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, 
	uint32_t numHandles, 
	uint32_t descriptorSize, 
	uint64 frameNumber,
	std::shared_ptr<DescriptorAllocatorPage> pPage) 
: _numDescriptors(numHandles), _descriptorSize(descriptorSize)
, _descriptor(descriptor), _frameNumber(frameNumber), _page(pPage)
{}

DescriptorAllocation::DescriptorAllocation(DescriptorAllocation &&other) noexcept
: _numDescriptors(other._numDescriptors), _descriptorSize(other._descriptorSize)
, _descriptor(other._descriptor), _page(other._page) 
{
	other._numDescriptors = 0;
	other._descriptorSize = 0;
	other._descriptor = D3D12_CPU_DESCRIPTOR_HANDLE(0);
	other._page = nullptr;
}

DescriptorAllocation &DescriptorAllocation::operator=(DescriptorAllocation &&other) noexcept {
	DescriptorAllocation tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

DescriptorAllocation::~DescriptorAllocation() {
	free();
}

bool DescriptorAllocation::isNull() const noexcept {
	return _descriptor.ptr == 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::getDescriptorHandle(uint32_t offset /*= 0*/) const noexcept {
	assert(offset < _numDescriptors);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_descriptor);
	handle.Offset(offset, _descriptorSize);
	return handle;
}

uint32_t DescriptorAllocation::getNumHandles() const noexcept {
	return _numDescriptors;
}

uint64 DescriptorAllocation::getFrameNumber() const noexcept {
	return _frameNumber;
}

auto DescriptorAllocation::getDescriptorAllocatorPage() const noexcept {
	return _page;
}

void DescriptorAllocation::free() {
	if (isNull() || _page == nullptr)
		return;
	_page->free(std::move(*this), _frameNumber);
}

void swap(DescriptorAllocation &lhs, DescriptorAllocation &rhs) noexcept {
	using std::swap;
	swap(lhs._numDescriptors, rhs._numDescriptors);
	swap(lhs._descriptor, rhs._descriptor);
	swap(lhs._descriptorSize, rhs._descriptorSize);
	swap(lhs._page, rhs._page);
}


DescriptorAllocatorPage::DescriptorAllocatorPage(ID3D12Device *pDevice, 
	D3D12_DESCRIPTOR_HEAP_TYPE type, 
	uint32 numDescriptors)
: _heapType(type), _numDescriptorInHeap(numDescriptors)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = _heapType;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = _numDescriptorInHeap;

	ThrowIfFailed(pDevice->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&_pDescriptorHeap)
	));

	_baseDescriptor = _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	_descriptorHandleIncrmenetSize = pDevice->GetDescriptorHandleIncrementSize(_heapType);
	_numFreeHandles = _numDescriptorInHeap;

	addNewBlock(0, _numFreeHandles);
}

bool DescriptorAllocatorPage::hasSpace(uint32 numDescriptors) const noexcept {
	return _freeListBySize.lower_bound(numDescriptors) != _freeListBySize.end();
}

DescriptorAllocation DescriptorAllocatorPage::allocate(uint32 numDescriptors, uint64 frameNumber) {
	std::lock_guard lock(_allocationMutex);
	if (numDescriptors > _numFreeHandles)
		return {};

	auto iter = _freeListBySize.lower_bound(numDescriptors);
	if (iter == _freeListBySize.end())
		return {};

	uint32 size = iter->first;
	uint32 offset = iter->second->first;
	auto descriptor = _baseDescriptor;
	descriptor.Offset(offset, _descriptorHandleIncrmenetSize);
	DescriptorAllocation ret = {
		static_cast<D3D12_CPU_DESCRIPTOR_HANDLE>(descriptor),
		numDescriptors,
		_descriptorHandleIncrmenetSize,
		frameNumber,
		this->shared_from_this(),
	};

	_freeListByOffset.erase(iter->second);
	_freeListBySize.erase(iter);
	uint32 newSize = size - numDescriptors;
	uint32 newOffset = offset + numDescriptors;
	_numFreeHandles -= numDescriptors;
	if (newSize > 0)
		addNewBlock(newOffset, newSize);
	
	return ret;
}

void DescriptorAllocatorPage::free(DescriptorAllocation &&handle, uint64 frameNumber) {
	DescriptorAllocation tmpHandle = std::move(handle);
	uint32 offset = computeOffset(tmpHandle.getDescriptorHandle());
	std::lock_guard lock(_allocationMutex);
	_staleDescriptorQueue.emplace(offset, tmpHandle.getNumHandles(), frameNumber);
}

uint32 DescriptorAllocatorPage::numFreeHandles() const noexcept {
	return _numFreeHandles;

}

void DescriptorAllocatorPage::releaseStaleDescriptor(uint64 frameNumber) {
	while (!_staleDescriptorQueue.empty()) {
		auto &front = _staleDescriptorQueue.front();
		if (front._frameNumber > frameNumber)
			break;
		freeBlock(front._offset, front._size);
	}
}

uint32 DescriptorAllocatorPage::computeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	auto offset = (handle.ptr - _baseDescriptor.ptr) / _descriptorHandleIncrmenetSize;
	return static_cast<uint32>(offset);
}

void DescriptorAllocatorPage::addNewBlock(uint32 offset, uint32 numDescriptors) {
	auto offsetIt = _freeListByOffset.emplace(offset, FreeBlockInfo{});
	auto sizeIt = _freeListBySize.emplace(numDescriptors, offsetIt.first);
	offsetIt.first->second._freeListBySizeIt = sizeIt;
}

void DescriptorAllocatorPage::freeBlock(uint32 offset, uint32 numDescriptors) {
	auto nextOffsetIt = _freeListByOffset.lower_bound(offset);
	auto newOffset = offset;
	auto newSize = numDescriptors;

	//        prev             curr               next
	// |----------------|------------------|--------------------|

	// try merge previource block
	if (nextOffsetIt != _freeListByOffset.begin()) {
		auto prevOffsetIt = nextOffsetIt;
		--prevOffsetIt;
		if (prevOffsetIt->first + prevOffsetIt->second._size == offset) {	// 中间是连续的块
			newOffset -= prevOffsetIt->first;
			newSize += prevOffsetIt->second._size;
			_freeListBySize.erase(prevOffsetIt->second._freeListBySizeIt);
			_freeListByOffset.erase(prevOffsetIt);
		}
	}

	// try merge next block
	if (nextOffsetIt != _freeListByOffset.end()) {
		if ((offset + numDescriptors == nextOffsetIt->first))				// 中间是连续的块
		newSize += nextOffsetIt->second._size;
		_freeListBySize.erase(nextOffsetIt->second._freeListBySizeIt);
		_freeListByOffset.erase(nextOffsetIt);
	}

	addNewBlock(newOffset, newSize);
}

DescriptorAllocator::DescriptorAllocator(ID3D12Device *pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPreHeap)
: _pDevice(pDevice), _numDescriptorsPreHeap(numDescriptorsPreHeap)
, _heapType(type) 
{}

DescriptorAllocation DescriptorAllocator::allocate(uint32 numDescriptors, uint64 frameNumber) {
	for (auto &pPage : _heapPool) {
		if (pPage->hasSpace(numDescriptors))
			return pPage->allocate(numDescriptors, frameNumber);
	}

	auto pPage = std::make_shared<DescriptorAllocatorPage>(_pDevice, _heapType, _numDescriptorsPreHeap);
	_heapPool.emplace_back(pPage);
	return pPage->allocate(numDescriptors, frameNumber);
}


void DescriptorAllocator::releaseStaleDescriptors(uint64 frameNumber) {
	for (auto &pPage : _heapPool)
		pPage->releaseStaleDescriptor(frameNumber);
}


}