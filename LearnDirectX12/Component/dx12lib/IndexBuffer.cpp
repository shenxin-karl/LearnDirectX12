#include "IndexBuffer.h"
#include "DefaultBuffer.h"

namespace dx12lib {

IndexBuffer::IndexBuffer() : _indexFormat(DXGI_FORMAT_UNKNOWN), _indexBufferByteSize(0) {
}

IndexBuffer::IndexBuffer(ID3D12Device *pDevice, 
	ID3D12GraphicsCommandList *pCmdList, 
	const void *pData, 
	uint32 sizeInByte, 
	DXGI_FORMAT format) 
: _indexFormat(format), _indexBufferByteSize(sizeInByte) 
{
	ThrowIfFailed(D3DCreateBlob(sizeInByte, &_pCPUBuffer));
	memcpy(_pCPUBuffer->GetBufferPointer(), pData, sizeInByte);
	_pGPUBuffer = std::make_unique<DefaultBuffer>(pDevice, pCmdList, pData, sizeInByte);
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept : IndexBuffer() {
	swap(*this, other);
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::getIndexBufferView() const noexcept {
	return {
		_pGPUBuffer->getAddress(),
		_indexBufferByteSize,
		_indexFormat
	};
}

DXGI_FORMAT IndexBuffer::getIndexFormat() const noexcept {
	return _indexFormat;
}

uint32 IndexBuffer::getIndexBufferSize() const noexcept {
	return _indexBufferByteSize;
}

bool IndexBuffer::isEmpty() const noexcept {
	return _pGPUBuffer == nullptr;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept {
	IndexBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(IndexBuffer &lhs, IndexBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pGPUBuffer, rhs._pGPUBuffer);
	swap(lhs._pCPUBuffer, rhs._pCPUBuffer);
	swap(lhs._indexFormat, rhs._indexFormat);
	swap(lhs._indexBufferByteSize, rhs._indexBufferByteSize);
}


}