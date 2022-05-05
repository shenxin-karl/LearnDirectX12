#include <dx12lib/Texture/DepthStencilTexture.h>
#include <dx12lib/Resource/ResourceStateTracker.h>
#include <dx12lib/Device/Device.h>

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> DepthStencil2D::getD3DResource() const {
	return _pResource;
}

ShaderResourceView DepthStencil2D::getSRV(size_t mipSlice) const {
	assert(mipSlice == 0);
	return _shaderResourceView;
}

DepthStencilView DepthStencil2D::getDSV() const {
	return _depthStencilView;
}

DepthStencil2D::~DepthStencil2D() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

DepthStencil2D::DepthStencil2D(std::weak_ptr<Device> pDevice, 
	size_t width,
	size_t height,
	const D3D12_CLEAR_VALUE *pClearValue,
	DXGI_FORMAT depthStencilFormat)
{
	auto pSharedDevice = pDevice.lock();
	if (depthStencilFormat == DXGI_FORMAT_UNKNOWN)
		depthStencilFormat = pSharedDevice->getDesc().depthStencilFormat;

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = static_cast<UINT>(height);
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(depthStencilDesc),
		D3D12_RESOURCE_STATE_COMMON,
		pClearValue,
		IID_PPV_ARGS(&_pResource)
	));
	createViews(pDevice);
}

DepthStencil2D::DepthStencil2D(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource, 
	D3D12_RESOURCE_STATES state) 
{
	_pResource = pResource;
	createViews(pDevice);
	ResourceStateTracker::addGlobalResourceState(pResource.Get(), state);
}

void DepthStencil2D::createViews(std::weak_ptr<Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	_depthStencilView = DepthStencilView(pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));
	pSharedDevice->getD3DDevice()->CreateDepthStencilView(
		_pResource.Get(),
		nullptr,
		_depthStencilView
	);

	auto desc = _pResource->GetDesc();
	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
	formatSupport.Format = desc.Format;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CheckFeatureSupport(
		D3D12_FEATURE_FORMAT_SUPPORT,
		&formatSupport,
		sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)
	));

	if (formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) {
		_shaderResourceView = ShaderResourceView(pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pResource.Get(),
			nullptr,
			_shaderResourceView
		);
	}
}

}
