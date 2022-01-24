#include "Texture.h"
#include "Device.h"

namespace dx12lib {

void Texture::resize(uint32 width, uint32 height, uint32 depthOrArraySize /*= 1*/) {
	if (_pResource == nullptr) {
		assert(false);
		return;
	}
	
	if (_width == width && _height == height && _depthOrArraySize == depthOrArraySize)
		return;

	CD3DX12_RESOURCE_DESC resDesc(_pResource->GetDesc());
	resDesc.Width = std::max(1u, width);
	resDesc.Height = std::max(1u, height);
	resDesc.DepthOrArraySize = depthOrArraySize;
	resDesc.MipLevels = resDesc.SampleDesc.Count > 1 ? 1 : 0;
	auto pd3d12Device = _pDevice.lock()->getD3DDevice();
	ThrowIfFailed(pd3d12Device->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COMMON,
		_pClearValue.get(),
		IID_PPV_ARGS(&_pResource)
	));
	// todo setName
	createViews();
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::getRenderTargetView() const {
	assert(checkRTVSupport());
	return _renderTargetView.getCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::getDepthStencilView() const {
	assert(checkDSVSupport());
	return _depthStencilView.getCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::getShaderResourceView() const {
	assert(checkSRVSupport());
	return _shaderResourceView.getCPUHandle();
}

bool Texture::checkRTVSupport() const noexcept {
	return checkFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET);
}

bool Texture::checkDSVSupport() const noexcept {
	return checkFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL);
}

bool Texture::checkSRVSupport() const noexcept {
	return checkFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE);
}

D3D12_RESOURCE_DESC Texture::getResourceDesc() const noexcept {
	D3D12_RESOURCE_DESC desc;
	std::memset(&desc, 0, sizeof(desc));
	if (_pResource != nullptr)
		desc = _pResource->GetDesc();
	return desc;
}

WRL::ComPtr<ID3D12Resource> Texture::getResource() const {
	return _pResource;
}

uint32 Texture::getWidth() const noexcept {
	return _width;
}

uint32 Texture::getHeight() const noexcept {
	return _height;
}

uint32 Texture::getDepthOrArraySize() const noexcept {
	return _depthOrArraySize;
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

	_width = desc.Width;
	_height = desc.Height;
	_depthOrArraySize = desc.DepthOrArraySize;

	checkFeatureSupport();
	createViews();
}


Texture::Texture(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource, const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/) {
	_pDevice = pDevice;
	auto desc = pResource->GetDesc();
	_width = desc.Width;
	_height = desc.Height;
	_depthOrArraySize = desc.DepthOrArraySize;
	_pResource = _pResource;
	if (pClearValue != nullptr)
		_pClearValue = std::make_unique<D3D12_CLEAR_VALUE>(*pClearValue);

	checkFeatureSupport();
	createViews();
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

void Texture::createViews() {
	if (_pResource == nullptr)
		return;

	auto pSharedDevice = _pDevice.lock();
	CD3DX12_RESOURCE_DESC desc(_pResource->GetDesc());
	if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 && checkRTVSupport()) {
		_renderTargetView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		pSharedDevice->getD3DDevice()->CreateRenderTargetView(
			_pResource.Get(), 
			nullptr, 
			_renderTargetView.getCPUHandle()
		);
	}
	if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 && checkDSVSupport()) {
		_depthStencilView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		pSharedDevice->getD3DDevice()->CreateDepthStencilView(
			_pResource.Get(),
			nullptr,
			_depthStencilView.getCPUHandle()
		);
	}
	if ((desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != 0 && checkSRVSupport()) {
		_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pResource.Get(),
			nullptr,
			_shaderResourceView.getCPUHandle()
		);
	}
}

}