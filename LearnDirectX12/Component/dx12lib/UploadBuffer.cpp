#include "UploadBuffer.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

UploadBuffer::UploadBuffer(ID3D12Device *pDevice, size_t elementCount, size_t elementByteSize, bool isConstantBuffer)
: _isConstantBuffer(isConstantBuffer), _elementByteSize(elementByteSize), _elementCount(elementCount), _pMappedData(nullptr)
{
	if (isConstantBuffer)
		_elementByteSize = calcConstantBufferByteSize(elementByteSize);

	ThrowIfFailed(pDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(elementByteSize) * _elementByteSize)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_pUploadResource)
	));

	ResourceStateTracker::addGlobalResourceState(_pUploadResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

UploadBuffer::UploadBuffer(UploadBuffer &&other) noexcept : UploadBuffer() {
	swap(*this, other);
}

UploadBuffer &UploadBuffer::operator=(UploadBuffer &&other) noexcept {
	UploadBuffer tmp;
	swap(*this, tmp);	
	swap(*this, other);
	return *this;
}

UploadBuffer::UploadBuffer() 
: _isConstantBuffer(false), _elementByteSize(0), _elementCount(0), _pMappedData(nullptr)
{
}

void UploadBuffer::map() const {
	assert(_pUploadResource != nullptr);
	if (_pMappedData == nullptr)
		_pUploadResource->Map(0, nullptr, reinterpret_cast<void **>(&_pMappedData));
}

void UploadBuffer::copyData(size_t elementIndex, const void *pData) {
	assert(elementIndex < _elementCount);
	map();
	auto *pDest = _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
	memcpy(pDest, pData, _elementByteSize);
}

void UploadBuffer::copyData(size_t elementIndex, const void* pData, size_t sizeInByte, size_t offset) {
	assert(elementIndex < _elementCount);
	assert((sizeInByte + offset) <= _elementByteSize);
	map();
	auto *pDest = _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
	memcpy(pDest + offset, pData, sizeInByte);
}

D3D12_GPU_VIRTUAL_ADDRESS UploadBuffer::getGPUAddressByIndex(size_t elementIndex /*= 0*/) const {
	D3D12_GPU_VIRTUAL_ADDRESS address = _pUploadResource->GetGPUVirtualAddress();
	address += static_cast<size_t>(elementIndex) * _elementByteSize;
	return address;
}

BYTE *UploadBuffer::getMappedDataByIndex(size_t elementIndex /*= 0*/) {
	assert(elementIndex < _elementCount);
	map();
	return _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
}

const BYTE *UploadBuffer::getMappedDataByIndex(size_t elementIndex /*= 0*/) const {
	assert(elementIndex < _elementCount);
	map();
	return _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
}

void UploadBuffer::unmap() const {
	if (_pUploadResource != nullptr && _pMappedData != nullptr) {
		_pUploadResource->Unmap(0, nullptr);
		_pMappedData = nullptr;
	}
}

size_t UploadBuffer::getElementByteSize() const noexcept {
	return _elementByteSize;
}

size_t UploadBuffer::getElementCount() const noexcept {
	return _elementCount;
}

WRL::ComPtr<ID3D12Resource> UploadBuffer::getD3DResource() const {
	return _pUploadResource;
}

UploadBuffer::~UploadBuffer() {
	unmap();
	ResourceStateTracker::removeGlobalResourceState(_pUploadResource.Get());
}

size_t UploadBuffer::calcConstantBufferByteSize(std::size_t size) noexcept {
	constexpr std::size_t mask = ~0xff;
	auto res = (size + 0xff) & mask;
	return res;
}

void swap(UploadBuffer &lhs, UploadBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pUploadResource, rhs._pUploadResource);
	swap(lhs._pMappedData, rhs._pMappedData);
	swap(lhs._elementByteSize, rhs._elementByteSize);
	swap(lhs._elementCount, rhs._elementCount);
	swap(lhs._isConstantBuffer, rhs._isConstantBuffer);
}


}