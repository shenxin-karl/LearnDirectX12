#include <dx12lib/Device/Device.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Resource/ResourceStateTracker.h>
#define SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <locale>

namespace dx12lib {

void IResource::setResourceName(const std::string &name) {
	static std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
	setResourceName(converter.from_bytes(name));
}

void IResource::setResourceName(const std::wstring &name) {
	WRL::ComPtr<ID3D12Resource> pResource = getD3DResource();
	assert(pResource != nullptr);
	pResource->SetName(name.c_str());
}

bool IShaderResource::checkSRVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ||
		   currentState & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
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

bool IRenderTarget2D::checkRTVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_RENDER_TARGET;
}

bool IRenderTarget2DArray::checkRTVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_RENDER_TARGET;
}

bool IRenderTargetCube::checkRTVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_RENDER_TARGET;
}

bool IUnorderedAccess2D::checkUAVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

bool IUnorderedAccess2DArray::checkUAVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

bool IUnorderedAccessCube::checkUAVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

bool IDepthStencil2D::checkDSVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

bool IDepthStencil2D::checkSRVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_DEPTH_READ;
}

BufferType IConstantBuffer::getBufferType() const {
	return BufferType::ConstantBuffer;
}

bool IConstantBuffer::checkCBVState(D3D12_RESOURCE_STATES currentState) const {
	return currentState & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
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

bool IStructuredBuffer::checkSRVState(D3D12_RESOURCE_STATES state) const {
	return state & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ||
		   state & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

BufferType IConsumeStructuredBuffer::getBufferType() const {
	return BufferType::ConsumeStructuredBuffer;
}

bool IConsumeStructuredBuffer::checkUAVState(D3D12_RESOURCE_STATES state) const {
	return state & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

BufferType IAppendStructuredBuffer::getBufferType() const {
	return BufferType::AppendStructuredBuffer;
}

bool IAppendStructuredBuffer::checkUAVState(D3D12_RESOURCE_STATES state) const {
	return state & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
}

BufferType IUAStructuredBuffer::getBufferType() const {
	return BufferType::StructuredBuffer;
}

bool IUAStructuredBuffer::checkUAVState(D3D12_RESOURCE_STATES state) const {
	return state & D3D12_RESOURCE_STATE_UNORDERED_ACCESS;	
}

bool IUAStructuredBuffer::checkSRVState(D3D12_RESOURCE_STATES state) const {
	return state & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ||
		   state & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

BufferType IReadBackBuffer::getBufferType() const {
	return BufferType::ReadBackBuffer;
}

}
