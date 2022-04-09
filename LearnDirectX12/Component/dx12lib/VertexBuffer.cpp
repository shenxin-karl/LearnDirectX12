#include "VertexBuffer.h"
#include "DefaultBuffer.h"
#include "CommandList.h"
#include "Device.h"

namespace dx12lib {

VertexBuffer::VertexBuffer() : _vertexStride(0) {
}

VertexBuffer::VertexBuffer(std::weak_ptr<Device> pDevice, std::shared_ptr<CommandList> pCmdList,
	const void *pData, size_t numElements, size_t stride)
: _vertexStride(stride)
{
	size_t sizeInByte = numElements * stride;
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(pDevice.lock()->getD3DDevice(), 
		pCmdList->getD3DCommandList(), 
		pData, 
		sizeInByte
	);
	_resourceType = ResourceType::VertexBuffer;
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept : VertexBuffer() {
	swap(*this, other);
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getVertexBufferView() const noexcept {
	return {
		_pDefaultBuffer->getAddress(),
		static_cast<UINT>(getVertexBufferSize()),
		static_cast<UINT>(_vertexStride)
	};
}

size_t VertexBuffer::getVertexBufferSize() const noexcept {
	return _pDefaultBuffer->getWidth();
}

size_t VertexBuffer::getVertexStride() const noexcept {
	return _vertexStride;
}

size_t VertexBuffer::getVertexCount() const noexcept {
	assert(_vertexStride != 0);
	return getVertexBufferSize() / _vertexStride;
}

bool VertexBuffer::isEmpty() const noexcept {
	return _pDefaultBuffer == nullptr;
}

WRL::ComPtr<ID3D12Resource> VertexBuffer::getD3DResource() const {
	if (_pDefaultBuffer == nullptr)
		return nullptr;
	return _pDefaultBuffer->getD3DResource();
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept {
	VertexBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(VertexBuffer &lhs, VertexBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._vertexStride, rhs._vertexStride);
	swap(lhs._pDefaultBuffer, rhs._pDefaultBuffer);
}

}