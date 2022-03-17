#include "IndexBuffer.h"
#include "DefaultBuffer.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

IndexBuffer::IndexBuffer() : _indexFormat(DXGI_FORMAT_UNKNOWN), _indexBufferByteSize(0) {
}

IndexBuffer::IndexBuffer(std::weak_ptr<Device> pDevice,
	std::shared_ptr<CommandList> pCmdList,
	const void *pData,
	uint32 sizeInByte,
	DXGI_FORMAT format)
: _indexFormat(format), _indexBufferByteSize(sizeInByte) 
{
	assert(getIndexStrideByFormat(format) != 0 && "invalid index type");
	//ThrowIfFailed(D3DCreateBlob(sizeInByte, &_pCPUBuffer));
	//memcpy(_pCPUBuffer->GetBufferPointer(), pData, sizeInByte);
	_pGPUBuffer = std::make_unique<DefaultBuffer>(
		pDevice.lock()->getD3DDevice(), 
		pCmdList->getD3DCommandList(), 
		pData, 
		sizeInByte
	);
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) noexcept : IndexBuffer() {
	swap(*this, other);
}

IndexBuffer::~IndexBuffer() {
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

uint32 IndexBuffer::getIndexCount() const noexcept {
	return _indexBufferByteSize / getIndexStrideByFormat(_indexFormat);
}

uint32 IndexBuffer::getIndexStrideByFormat(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_R8_UINT:
		return 1;
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_R16_UINT:
		return 2;
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R32_UINT:
		return 4;
	default:
		assert(false);
		return 0;
	}
}

bool IndexBuffer::isEmpty() const noexcept {
	return _pGPUBuffer == nullptr;
}

WRL::ComPtr<ID3D12Resource> IndexBuffer::getD3DResource() const {
	if (_pGPUBuffer == nullptr)
		return nullptr;
	return _pGPUBuffer->getD3DResource();
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
	//swap(lhs._pCPUBuffer, rhs._pCPUBuffer);
	swap(lhs._indexFormat, rhs._indexFormat);
	swap(lhs._indexBufferByteSize, rhs._indexBufferByteSize);
}


}