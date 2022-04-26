#include "RenderTargetBuffer.h"
#include "Device.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> RenderTargetBuffer::getD3DResource() const {
	return _pResource;
}

RenderTargetBuffer::~RenderTargetBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

RenderTargetView RenderTargetBuffer::getRenderTargetView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_rtvDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&_srvDesc,
		descriptor.getCPUHandle()
	);
	return RenderTargetView(descriptor);
}

ShaderResourceView RenderTargetBuffer::getShaderResourceView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_srvDesc.Texture2D.MostDetailedMip = static_cast<UINT>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&_srvDesc,
		descriptor.getCPUHandle()
	);
	return ShaderResourceView(descriptor);
}

RenderTargetBuffer::RenderTargetBuffer(std::weak_ptr<Device> 
	pDevice, WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
	initViewDesc(pResource->GetDesc().Format);
	_resourceType = ResourceType::RenderTargetBuffer;
}

RenderTargetBuffer::RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
	uint32 width, 
	uint32 height, 
	D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/, 
	DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN */) : _rtvDesc({}), _srvDesc({})
{
	D3D12_RESOURCE_DESC renderTargetDesc;
	renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDesc.Alignment = 0;
	renderTargetDesc.Width = width;
	renderTargetDesc.Height = height;
	renderTargetDesc.DepthOrArraySize = 1;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Format = format;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(renderTargetDesc),
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));

	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
	initViewDesc(format);
	_resourceType = ResourceType::RenderTargetBuffer;
}

void RenderTargetBuffer::initViewDesc(DXGI_FORMAT format) {
	_rtvDesc.Format = format;
	_rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	_rtvDesc.Texture2D.PlaneSlice = 0;
	_rtvDesc.Texture2D.MipSlice = 0;

	_srvDesc.Format = format;
	_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	_srvDesc.Texture2D.MostDetailedMip = 0;
	_srvDesc.Texture2D.MipLevels = 1;
	_srvDesc.Texture2D.PlaneSlice = 0;
	_srvDesc.Texture2D.ResourceMinLODClamp = 0;
}

}
