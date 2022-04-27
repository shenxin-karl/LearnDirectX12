#include "RenderTargetBuffer.h"
#include "Device.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> RenderTarget2D::getD3DResource() const {
	return _pResource;
}

RenderTarget2D::~RenderTarget2D() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

RenderTargetView RenderTarget2D::getRenderTargetView(size_t mipSlice) const {
	if (_rtvMgr.exist(mipSlice))
		return _rtvMgr.get(mipSlice);

	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = _pResource->GetDesc().Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.PlaneSlice = 0;
	rtvDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateRenderTargetView(
		_pResource.Get(),
		&rtvDesc,
		descriptor.getCPUHandle()
	);

	RenderTargetView RTV(descriptor);
	_rtvMgr.set(mipSlice, RTV);
	return RTV;
}

ShaderResourceView RenderTarget2D::getShaderResourceView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	if (_srvMgr.exist(mipSlice))
		return _srvMgr.get(mipSlice);

	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = _pResource->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = static_cast<float>(mipSlice);

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		descriptor.getCPUHandle()
	);
	
	ShaderResourceView SRV(descriptor);
	_srvMgr.set(mipSlice, SRV);
	return SRV;
}

ShaderResourceType RenderTarget2D::getShaderResourceType() const {
	return ShaderResourceType::RenderTarget2D;
}

ResourceType RenderTarget2D::getResourceType() const {
	return ResourceType::RenderTargetBuffer | ResourceType::ShaderResourceBuffer;
}

RenderTarget2D::RenderTarget2D(std::weak_ptr<Device> 
                                       pDevice, WRL::ComPtr<ID3D12Resource> pResource,
                                       D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource)
{
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}

RenderTarget2D::RenderTarget2D(std::weak_ptr<Device> pDevice, 
	uint32 width, 
	uint32 height, 
	D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/, 
	DXGI_FORMAT format /*= DXGI_FORMAT_UNKNOWN */)
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
}

}
