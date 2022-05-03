#include "VertexBuffer.h"
#include "DefaultBuffer.h"
#include "CommandList.h"
#include "Device.h"

namespace dx12lib {

VertexBuffer::VertexBuffer(std::weak_ptr<Device> pDevice, 
	std::shared_ptr<CommandList> pCmdList,
	const void *pData, 
	size_t numElements, 
	size_t stride)
: _vertexStride(stride)
{
	size_t sizeInByte = numElements * stride;
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(pDevice.lock()->getD3DDevice(), 
		pCmdList->getD3DCommandList(), 
		pData, 
		sizeInByte
	);
}

WRL::ComPtr<ID3D12Resource> VertexBuffer::getD3DResource() const {
	return _pDefaultBuffer->getD3DResource();
}

size_t VertexBuffer::getBufferSize() const {
	return _pDefaultBuffer->getBufferSize();
}

size_t VertexBuffer::getVertexCount() const {
	assert(_vertexStride != 0);
	return getBufferSize() / _vertexStride;
}

size_t VertexBuffer::getVertexStride() const {
	return _vertexStride;
}

VertexBufferView VertexBuffer::getVertexBufferView() const {
	return VertexBufferView(
		_pDefaultBuffer->getAddress(), 
		static_cast<UINT>(getBufferSize()), 
		static_cast<UINT>(_vertexStride)
	);
}

}