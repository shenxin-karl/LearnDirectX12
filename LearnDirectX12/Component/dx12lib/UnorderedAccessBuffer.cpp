#include "UnorderedAccessBuffer.h"
#include "Device.h"
#include "D3Dx12.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> UnorderedAccessBuffer::getD3DResource() const {
	return _pResource;
}

UnorderedAccessView UnorderedAccessBuffer::getUnorderedAccessView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_uavDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		&_uavDesc,
		descriptor.getCPUHandle()
	);
	return UnorderedAccessView(descriptor);
}

ShaderResourceView UnorderedAccessBuffer::getShaderResourceView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_uavDesc.Texture2D.MipSlice = static_cast<UINT>(mipSlice);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&_srvDesc,
		descriptor.getCPUHandle()
	);
	return ShaderResourceView(descriptor);
}

ShaderResourceType UnorderedAccessBuffer::getShaderResourceType() const {
	return ShaderResourceType::UnorderedAccess;
}

ResourceType UnorderedAccessBuffer::getResourceType() const {
	return ResourceType::UnorderedAccessBuffer | ResourceType::ShaderResourceBuffer;
}

std::size_t UnorderedAccessBuffer::getBufferSize() const {
	return _pResource->GetDesc().Width;
}

UnorderedAccessBuffer::~UnorderedAccessBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}


UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
	std::size_t width, std::size_t height,
	DXGI_FORMAT format)
: _uavDesc({}), _srvDesc({}), _pDevice(pDevice)
{
	D3D12_RESOURCE_DESC uavDesc = {};
	uavDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	uavDesc.Alignment = 0;
	uavDesc.Width = width;
	uavDesc.Height = static_cast<UINT>(height);
	uavDesc.DepthOrArraySize = 1;
	uavDesc.MipLevels = 1;
	uavDesc.Format = format;
	uavDesc.SampleDesc.Count = 1;
	uavDesc.SampleDesc.Quality = 0;
	uavDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	uavDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(uavDesc),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
	initViewDesc(format);
}

UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state) 
{
	assert(state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	_pResource = pResource;
	auto format = pResource->GetDesc().Format;
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
	initViewDesc(format);
}

void UnorderedAccessBuffer::initViewDesc(DXGI_FORMAT format) {
	_uavDesc.Format = format;
	_uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	_uavDesc.Texture2D.MipSlice = 0;
	_uavDesc.Texture2D.PlaneSlice = 0;

	_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	_srvDesc.Format = format;
	_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	_srvDesc.Texture2D.MostDetailedMip = 0;
	_srvDesc.Texture2D.MipLevels = 1;
}

}
