#include <dx12lib/Device/Device.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Resource/ResourceStateTracker.h>

namespace dx12lib {

void IResource::setResourceName(const std::string &name) {
	setResourceName(std::to_wstring(name));
}

void IResource::setResourceName(const std::wstring &name) {
	WRL::ComPtr<ID3D12Resource> pResource = getD3DResource();
	assert(pResource != nullptr);
	pResource->SetName(name.c_str());
}

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

ShaderResourceDimension IShaderResource2D::getDimension() const {
	return ShaderResourceDimension::Texture2D;
}

ShaderResourceDimension IShaderResource2DArray::getDimension() const {
	return ShaderResourceDimension::Texture2DArray;
}

size_t IShaderResource2DArray::getPlaneSlice() const {
	return getD3DResource()->GetDesc().DepthOrArraySize;
}

ShaderResourceDimension IShaderResourceCube::getDimension() const {
	return ShaderResourceDimension::TextureCube;
}

BufferType IConstantBuffer::getBufferType() const {
	return BufferType::ConstantBuffer;
}

BufferType IVertexBuffer::getBufferType() const {
	return BufferType::IndexBuffer;
}

BufferType IIndexBuffer::getBufferType() const {
	return BufferType::IndexBuffer;
}

BufferType IStructuredBuffer::getBufferType() const {
	return BufferType::StructuredBuffer;
}

BufferType IReadBackBuffer::getBufferType() const {
	return BufferType::ReadBackBuffer;
}

}
