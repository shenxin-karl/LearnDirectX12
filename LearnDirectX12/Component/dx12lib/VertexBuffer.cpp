#include "VertexBuffer.h"
#include "DefaultBuffer.h"

namespace dx12lib {

VertexBuffer::VertexBuffer() : _bufferByteSize(0), _vertexStride(0) {
}

VertexBuffer::VertexBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, void *pData, uint32 sizeInByte, uint32 stride)
: _bufferByteSize(sizeInByte), _vertexStride(stride)
{
	_pGPUBuffer = std::make_unique<DefaultBuffer>(pDevice, pCmdList, pData, sizeInByte);
	ThrowIfFailed(D3DCreateBlob(sizeInByte, &_pCPUBuffer));
	memcpy(_pCPUBuffer->GetBufferPointer(), pData, sizeInByte);
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept : VertexBuffer() {
	swap(*this, other);
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::getVertexBufferView() const noexcept {
	return {
		_pGPUBuffer->getAddress(),
		_bufferByteSize,
		_vertexStride
	};
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) noexcept {
	VertexBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(VertexBuffer &lhs, VertexBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._bufferByteSize, rhs._bufferByteSize);
	swap(lhs._vertexStride, rhs._vertexStride);
	swap(lhs._pGPUBuffer, rhs._pGPUBuffer);
	swap(lhs._pCPUBuffer, rhs._pCPUBuffer);
}

}