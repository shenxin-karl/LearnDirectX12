#include "VertexBuffer.h"
#include "DefaultBuffer.h"
#include "CommandList.h"
#include "Device.h"

namespace dx12lib {

VertexBuffer::VertexBuffer() : _bufferByteSize(0), _vertexStride(0) {
}

VertexBuffer::VertexBuffer(std::weak_ptr<Device> pDevice, std::shared_ptr<CommandList> pCmdList,
	const void *pData, uint32 sizeInByte, uint32 stride)
: _bufferByteSize(sizeInByte), _vertexStride(stride)
{
	_pGPUBuffer = std::make_unique<DefaultBuffer>(pDevice.lock()->getD3DDevice(), 
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
		_pGPUBuffer->getAddress(),
		_bufferByteSize,
		_vertexStride
	};
}

//WRL::ComPtr<ID3DBlob> VertexBuffer::getCPUBuffer() const noexcept {
//	return _pCPUBuffer;
//}

uint32 VertexBuffer::getVertexBufferSize() const noexcept {
	return _bufferByteSize;
}

uint32 VertexBuffer::getVertexStride() const noexcept {
	return _vertexStride;
}

uint32 VertexBuffer::getVertexCount() const noexcept {
	assert(_vertexStride != 0);
	return _bufferByteSize / _vertexStride;
}

bool VertexBuffer::isEmpty() const noexcept {
	return _pGPUBuffer == nullptr;
}

WRL::ComPtr<ID3D12Resource> VertexBuffer::getD3DResource() const {
	if (_pGPUBuffer == nullptr)
		return nullptr;
	return _pGPUBuffer->getD3DResource();
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
	//swap(lhs._pCPUBuffer, rhs._pCPUBuffer);
}

}