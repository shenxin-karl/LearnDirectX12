#include "UploadBuffer.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

UploadBuffer::UploadBuffer(ID3D12Device *pDevice, size_t elementCount, size_t elementByteSize, bool isConstantBuffer)
: _isConstantBuffer(isConstantBuffer), _elementByteSize(elementByteSize), _elementCount(elementCount)
{
	if (isConstantBuffer)
		_elementByteSize = calcConstantBufferByteSize(elementByteSize);

	ThrowIfFailed(pDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(elementByteSize) * _elementByteSize)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_pUploadBuffer)
	));

	_pUploadBuffer->Map(0, nullptr, reinterpret_cast<void **>(&_pMappedData));
	ResourceStateTracker::addGlobalResourceState(_pUploadBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
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
: _pMappedData(nullptr), _isConstantBuffer(false), _elementByteSize(0), _elementCount(0)
{
}

void UploadBuffer::copyData(size_t elementIndex, const void *pData) {
	assert(elementIndex < _elementCount);
	auto *pDest = _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
	memcpy(pDest, pData, _elementByteSize);
}

void UploadBuffer::copyData(size_t elementIndex, const void* pData, size_t sizeInByte, size_t offset) {
	assert(elementIndex < _elementCount);
	assert((sizeInByte + offset) <= _elementByteSize);
	auto *pDest = _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
	memcpy(pDest + offset, pData, sizeInByte);
}

D3D12_GPU_VIRTUAL_ADDRESS UploadBuffer::getGPUAddressByIndex(size_t elementIndex /*= 0*/) const {
	D3D12_GPU_VIRTUAL_ADDRESS address = _pUploadBuffer->GetGPUVirtualAddress();
	address += static_cast<size_t>(elementIndex) * _elementByteSize;
	return address;
}

BYTE *UploadBuffer::getMappedDataByIndex(size_t elementIndex /*= 0*/) {
	assert(elementIndex < _elementCount);
	return _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
}

const BYTE *UploadBuffer::getMappedDataByIndex(size_t elementIndex /*= 0*/) const {
	assert(elementIndex < _elementCount);
	return _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
}

size_t UploadBuffer::getElementByteSize() const noexcept {
	return _elementByteSize;
}

size_t UploadBuffer::getElementCount() const noexcept {
	return _elementCount;
}

WRL::ComPtr<ID3D12Resource> UploadBuffer::getD3DResource() const {
	return _pUploadBuffer;
}

UploadBuffer::~UploadBuffer() {
	if (_pUploadBuffer != nullptr) {
		_pUploadBuffer->Unmap(0, nullptr);
		ResourceStateTracker::removeGlobalResourceState(_pUploadBuffer.Get());
	}
}

size_t UploadBuffer::calcConstantBufferByteSize(std::size_t size) noexcept {
	constexpr std::size_t mask = ~0xff;
	auto res = (size + 0xff) & mask;
	return res;
}

void swap(UploadBuffer &lhs, UploadBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pUploadBuffer, rhs._pUploadBuffer);
	swap(lhs._pMappedData, rhs._pMappedData);
	swap(lhs._elementByteSize, rhs._elementByteSize);
	swap(lhs._elementCount, rhs._elementCount);
	swap(lhs._isConstantBuffer, rhs._isConstantBuffer);
}


}