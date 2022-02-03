#include "Texture.h"
#include "Device.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

ClearFlag operator|(ClearFlag lhs, ClearFlag rhs) {
	std::size_t flag = static_cast<std::size_t>(lhs) | static_cast<std::size_t>(rhs);
	return ClearFlag(flag);
}

ClearFlag &operator|=(ClearFlag &lhs, ClearFlag rhs) {
	lhs = lhs | rhs;
	return lhs;
}


bool operator&(ClearFlag lhs, ClearFlag rhs) {
	return static_cast<std::size_t>(lhs) & static_cast<std::size_t>(rhs);
}

void Texture::resize(uint32 width, uint32 height, uint32 depthOrArraySize /*= 1*/) {
	if (getD3DResource() == nullptr) {
		assert(false);
		return;
	}
	
	if (_width == width && _height == height && _depthOrArraySize == depthOrArraySize)
		return;

	CD3DX12_RESOURCE_DESC resDesc(getD3DResource()->GetDesc());
	resDesc.Width = std::max(1u, width);
	resDesc.Height = std::max(1u, height);
	resDesc.DepthOrArraySize = depthOrArraySize;
	resDesc.MipLevels = resDesc.SampleDesc.Count > 1 ? 1 : 0;
	auto pd3d12Device = _pDevice.lock()->getD3DDevice();
	WRL::ComPtr<ID3D12Resource> pResource;
	ThrowIfFailed(pd3d12Device->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&_clearValue,
		IID_PPV_ARGS(&pResource)
	));
	setD3DResource(pResource);
	checkFeatureSupport(pd3d12Device);
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
	if (auto pResource = getD3DResource())
		desc = pResource->GetDesc();
	return desc;
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

const D3D12_CLEAR_VALUE *Texture::getClearValue() const {
	return &_clearValue;
}

D3D12_CLEAR_VALUE *Texture::getClearValue() {
	return &_clearValue;
}

void Texture::clearColor(DX::XMVECTORF32 color) {
	memcpy(&_clearValue.Color, &color, sizeof(_clearValue.Color));
	_clearFlag |= ClearFlag::Color;
}

void Texture::clearDepth(float depth) {
	_clearValue.DepthStencil.Depth = depth;
	_clearFlag |= ClearFlag::Depth;
}

void Texture::clearStencil(UINT stencil) {
	_clearValue.DepthStencil.Stencil = stencil;
	_clearFlag |= ClearFlag::Stencil;
}

void Texture::setClearFlag(ClearFlag flag) {
	_clearFlag = flag;
}

void Texture::clearDepthStencil(float depth, UINT stencil) {
	clearDepth(depth);
	clearStencil(stencil);
}

void Texture::clearColorDepthStencil(DX::XMVECTORF32 color, float depth, UINT stencil) {
	clearColor(color);
	clearDepth(depth);
	clearStencil(stencil);
}

ClearFlag Texture::getClearFlag() const {
	return _clearFlag;
}

Texture::Texture(std::weak_ptr<Device> pDevice, const D3D12_RESOURCE_DESC &desc, 
	const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/) 
: Resource(pDevice.lock()->getD3DDevice(), desc, pClearValue)
{
	_pDevice = pDevice;
	initializeClearValue(pClearValue);
	_width = static_cast<uint32>(desc.Width);
	_height = static_cast<uint32>(desc.Height);
	_depthOrArraySize = desc.DepthOrArraySize;
	createViews();
}


Texture::Texture(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource, const D3D12_CLEAR_VALUE *pClearValue /*= nullptr*/)
: Resource(pDevice.lock()->getD3DDevice(), pResource)
{
	_pDevice = pDevice;
	auto desc = pResource->GetDesc();
	_width = static_cast<uint32>(desc.Width);
	_height = static_cast<uint32>(desc.Height);
	_depthOrArraySize = desc.DepthOrArraySize;
	initializeClearValue(pClearValue);
	createViews();
}

void Texture::initializeClearValue(const D3D12_CLEAR_VALUE *pClearValue) {
	if (pClearValue != nullptr) {
		_clearValue = *pClearValue;
	} else {
		std::memset(&_clearValue.Color, 0, sizeof(_clearValue.Color));
		_clearValue.DepthStencil.Depth = 1.f;
		_clearValue.DepthStencil.Stencil = 0;
	}
	_clearFlag = ClearFlag::Color | ClearFlag::Depth | ClearFlag::Stencil;
}

void Texture::createViews() {
	auto pResource = getD3DResource();
	if (pResource == nullptr)
		return;

	auto pSharedDevice = _pDevice.lock();
	CD3DX12_RESOURCE_DESC desc(pResource->GetDesc());
	if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 && checkRTVSupport()) {
		_renderTargetView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		pSharedDevice->getD3DDevice()->CreateRenderTargetView(
			pResource.Get(),
			nullptr,
			_renderTargetView.getCPUHandle()
		);
	}
	if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 && checkDSVSupport()) {
		_depthStencilView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		pSharedDevice->getD3DDevice()->CreateDepthStencilView(
			pResource.Get(),
			nullptr,
			_depthStencilView.getCPUHandle()
		);
	}
	if ((desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) != 0 && checkSRVSupport()) {
		_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			pResource.Get(),
			nullptr,
			_shaderResourceView.getCPUHandle()
		);
	}
}

}