#include "Device.h"
#include "IResource.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

size_t IShaderResource::getWidth() const {
	return static_cast<size_t>(getD3DResource()->GetDesc().Width);
}

size_t IShaderResource::getHeight() const {
	return static_cast<size_t>(getD3DResource()->GetDesc().Height);
}

size_t IShaderResource::getMipmapLevels() const {
	return static_cast<size_t>(getD3DResource()->GetDesc().MipLevels);
}

DXGI_FORMAT IShaderResource::getFormat() const {
	return getD3DResource()->GetDesc().Format;
}

BufferType IConstantBuffer::getBufferType() const {
	return BufferType::ConstantBuffer;
}

BufferType IVertexBuffer::getBufferType() const {
	return BufferType::IndexBuffer;
}

BufferType IStructuredBuffer::getBufferType() const {
	return BufferType::StructuredBuffer;
}

BufferType IReadBackBuffer::getBufferType() const {
	return BufferType::ReadBackBuffer;
}

}
