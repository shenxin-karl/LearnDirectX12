#include "UploadBuffer.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

UploadBuffer::UploadBuffer(ID3D12Device *pDevice, UINT elementCount, UINT elementByteSize, bool isConstantBuffer)
: _elementCount(elementCount), _isConstantBuffer(isConstantBuffer)
{
	_elementByteSize = elementByteSize;
	if (isConstantBuffer)
		_elementByteSize = calcConstantBufferByteSize(elementByteSize);
	auto heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto buffer = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(elementByteSize) * _elementByteSize);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&buffer,
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
: _pMappedData(nullptr), _elementByteSize(0), _elementCount(0), _isConstantBuffer(false)
{
}

void UploadBuffer::copyData(UINT elementIndex, const void *pData) {
	assert(elementIndex < _elementCount);
	auto *pDest = _pMappedData + static_cast<std::ptrdiff_t>(elementIndex) * _elementByteSize;
	memcpy(pDest, pData, _elementByteSize);
}

D3D12_GPU_VIRTUAL_ADDRESS UploadBuffer::getGPUAddressByIndex(UINT elementIndex /*= 0*/) const {
	D3D12_GPU_VIRTUAL_ADDRESS address = _pUploadBuffer->GetGPUVirtualAddress();
	address += elementIndex;
	return address;
}

BYTE *UploadBuffer::getMappedDataByIndex(UINT elementIndex /*= 0*/) {
	assert(elementIndex < _elementCount);
	return _pMappedData + (std::ptrdiff_t(elementIndex) * _elementByteSize);
}

const BYTE *UploadBuffer::getMappedDataByIndex(UINT elementIndex /*= 0*/) const {
	assert(elementIndex < _elementCount);
	return _pMappedData + (std::ptrdiff_t(elementIndex) * _elementByteSize);
}

uint32 UploadBuffer::getElementByteSize() const noexcept {
	return _elementByteSize;
}

uint32 UploadBuffer::getElementCount() const noexcept {
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

UINT UploadBuffer::calcConstantBufferByteSize(std::size_t size) noexcept {
	constexpr std::size_t mask = ~0xff;
	auto res = (size + 0xff) & mask;
	return static_cast<UINT>(res);
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