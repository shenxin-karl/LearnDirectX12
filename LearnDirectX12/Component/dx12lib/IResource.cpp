#include "Device.h"
#include "IResource.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

uint64 IResource::getWidth() const {
	return getD3DResource()->GetDesc().Width;
}

uint64 IResource::getHeight() const {
	return getD3DResource()->GetDesc().Height;
}

uint64 IResource::getDepth() const {
	return getD3DResource()->GetDesc().DepthOrArraySize;
}

DXGI_FORMAT IResource::getFormat() const {
	return getD3DResource()->GetDesc().Format;
}

Resource::Resource(ID3D12Device *pDevice, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/) {
	ThrowIfFailed(pDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
	checkFeatureSupport(pDevice);
}

Resource::Resource(ID3D12Device *pDevice, WRL::ComPtr<ID3D12Resource> pResource, D3D12_RESOURCE_STATES state) 
: _pResource(pResource) 
{
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
	checkFeatureSupport(pDevice);
}

WRL::ComPtr<ID3D12Resource> Resource::getD3DResource() const {
	return _pResource;
}

void Resource::setD3DResource(WRL::ComPtr<ID3D12Resource> pResource, D3D12_RESOURCE_STATES state) {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
	_pResource = pResource;
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

void Resource::checkFeatureSupport(ID3D12Device *pDevice) {
	auto desc = _pResource->GetDesc();
	_formatSupport.Format = desc.Format;
	ThrowIfFailed(pDevice->CheckFeatureSupport(
		D3D12_FEATURE_FORMAT_SUPPORT,
		&_formatSupport,
		sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)
	));
}

bool Resource::checkFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const {
	return _formatSupport.Support1 & formatSupport;
}

Resource::~Resource() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

}