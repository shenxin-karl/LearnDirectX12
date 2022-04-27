#include "IndexBuffer.h"
#include "DefaultBuffer.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

IndexBuffer::IndexBuffer() : _indexFormat(DXGI_FORMAT_UNKNOWN) {
}

IndexBuffer::IndexBuffer(std::weak_ptr<Device> pDevice,
	std::shared_ptr<CommandList> pCmdList,
	const void *pData,
	size_t numElements,
	DXGI_FORMAT format)
: _indexFormat(format)
{
	auto stride = getIndexStrideByFormat(format);
	size_t sizeInByte = numElements * stride;
	assert(stride != 0 && "invalid index type");
	_pDefaultBUffer = std::make_unique<DefaultBuffer>(
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
		_pDefaultBUffer->getAddress(),
		static_cast<UINT>(getIndexBufferSize()),
		_indexFormat
	};
}

DXGI_FORMAT IndexBuffer::getIndexFormat() const noexcept {
	return _indexFormat;
}

size_t IndexBuffer::getIndexBufferSize() const noexcept {
	return _pDefaultBUffer->getWidth();
}

size_t IndexBuffer::getIndexCount() const noexcept {
	return getIndexBufferSize() / getIndexStrideByFormat(_indexFormat);
}

size_t IndexBuffer::getIndexStrideByFormat(DXGI_FORMAT format) {
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
	return _pDefaultBUffer == nullptr;
}

WRL::ComPtr<ID3D12Resource> IndexBuffer::getD3DResource() const {
	if (_pDefaultBUffer == nullptr)
		return nullptr;
	return _pDefaultBUffer->getD3DResource();
}

ResourceType IndexBuffer::getResourceType() const {
	return ResourceType::IndexBuffer;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) noexcept {
	IndexBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(IndexBuffer &lhs, IndexBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pDefaultBUffer, rhs._pDefaultBUffer);
	swap(lhs._indexFormat, rhs._indexFormat);
}


}