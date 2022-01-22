#include "Texture.h"
#include "Device.h"

namespace dx12lib {

void Texture::resize(uint32 width, uint32 height, uint32 depthOrArraySize /*= 1*/) {
	if (_pResource == nullptr)
		return;
	
}

Texture::Texture(std::weak_ptr<Device> pDevice, const D3D12_RESOURCE_DESC &desc, 
	const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/) 
{
	auto pD3DDevice = pDevice.lock()->getD3DDevice();
	if (pClearValue != nullptr)
		_pClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*pClearValue);

	ThrowIfFailed(pD3DDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)), 
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		_pClearValue.get(),
		IID_PPV_ARGS(&_pResource)
	));


}

void Texture::checkFeatureSupport() {
	auto pD3DDevice = _pDevice.lock()->getD3DDevice();
	auto desc = _pResource->GetDesc();
	_formatSupport.Format = desc.Format;
	ThrowIfFailed(pD3DDevice->CheckFeatureSupport(
		D3D12_FEATURE_FORMAT_SUPPORT,
		&_formatSupport,
		sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)
	));
}

bool Texture::checkFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const {
	return _formatSupport.Support1 & formatSupport;
}

}