#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"

namespace dx12lib {

DescriptorAllocation::DescriptorAllocation() 
: _numHandle(0), _handleSize(0)
, _baseHandle(D3D12_CPU_DESCRIPTOR_HANDLE{ 0 }), _pPage(nullptr) {
}

DescriptorAllocation::DescriptorAllocation(DescriptorAllocation &&other) noexcept 
: DescriptorAllocation() 
{
	swap(*this, other);
}

void swap(DescriptorAllocation &lhs, DescriptorAllocation &rhs) noexcept {
	using std::swap;
	swap(lhs._numHandle, rhs._numHandle);
	swap(lhs._handleSize, rhs._handleSize);
	swap(lhs._baseHandle, rhs._baseHandle);
	swap(lhs._pPage, rhs._pPage);
}


DescriptorAllocation::~DescriptorAllocation() {
	free();
}

DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE handle,
	uint32 numHandle,
	uint32 handleSize,
	std::shared_ptr<DescriptorAllocatorPage> pPage)
: _numHandle(numHandle), _handleSize(handleSize), _baseHandle(handle), _pPage(pPage) {
}

DescriptorAllocation &DescriptorAllocation::operator=(DescriptorAllocation &&other) noexcept {
	DescriptorAllocation tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

uint32 DescriptorAllocation::getNumHandle() const noexcept {
	return _numHandle;
}

uint32 DescriptorAllocation::getHandleSize() const noexcept {
	return _handleSize;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::getBaseHandle() const noexcept {
	return _baseHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::getHandleByOffset(uint32 offset) const {
	assert(offset > _numHandle);
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_baseHandle);
	handle.Offset(offset, _handleSize);
	return handle;
}

bool DescriptorAllocation::isNull() const noexcept {
	return _baseHandle.ptr == 0;
}

bool DescriptorAllocation::isValid() const noexcept {
	return _baseHandle.ptr != 0;
}

void DescriptorAllocation::free() {
	if (isNull() || _pPage == nullptr)
		return;
	_pPage->free(std::move(*this));
}

void DescriptorAllocation::clear() noexcept {
	_numHandle = 0;
	_handleSize = 0;
	_baseHandle = D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };
	_pPage = nullptr;
}

}