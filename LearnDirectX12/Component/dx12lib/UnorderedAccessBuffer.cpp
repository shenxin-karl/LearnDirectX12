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
}

UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
	std::size_t sizeInByte, 
	DXGI_FORMAT format) 
{
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(
			sizeInByte,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		)),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&_pResource)
	));
	createViews(pDevice, format);
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
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