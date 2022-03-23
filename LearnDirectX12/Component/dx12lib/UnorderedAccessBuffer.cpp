#include "UnorderedAccessBuffer.h"
#include "Device.h"
#include "D3Dx12.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> UnorderedAccessBuffer::getD3DResource() const {
	return _pResource;
}

D3D12_CPU_DESCRIPTOR_HANDLE UnorderedAccessBuffer::getUnorderedAccessView() const {
	return _unorderedAccessView.getCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE UnorderedAccessBuffer::getShaderResourceView() const {
	assert(isShaderSample());
	return _shaderResourceView.getCPUHandle();
}

bool UnorderedAccessBuffer::isShaderSample() const {
	return _shaderResourceView.isValid();
}

std::size_t UnorderedAccessBuffer::getBufferSize() const {
	return _pResource->GetDesc().Width;
}

UnorderedAccessBuffer::~UnorderedAccessBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

void UnorderedAccessBuffer::createViews(std::weak_ptr<Device> pDevice, DXGI_FORMAT format) {
	auto pSharedDevice = pDevice.lock();
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	_unorderedAccessView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pResource.Get(),
		nullptr,
		nullptr,
		_unorderedAccessView.getCPUHandle()
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&srvDesc,
		_shaderResourceView.getCPUHandle()
	);
}

UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
	std::size_t width, std::size_t height,
	DXGI_FORMAT format) 
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
	createViews(pDevice, format);
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
}

UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource,
	D3D12_RESOURCE_STATES state) 
{
	assert(state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	_pResource = pResource;
	auto format = pResource->GetDesc().Format;
	createViews(pDevice, format);
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

}