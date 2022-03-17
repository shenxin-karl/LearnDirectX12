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

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetBuffer::getRenderTargetView() const {
	return _renderTargetView.getCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetBuffer::getShaderResourceView() const {
	assert(_shaderResourceView.isValid());
	return _shaderResourceView.getCPUHandle();
}

bool RenderTargetBuffer::isShaderSample() const {
	return _shaderResourceView.isValid();
}

void RenderTargetBuffer::createViews(std::weak_ptr<Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	_renderTargetView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	pSharedDevice->getD3DDevice()->CreateRenderTargetView(
		_pResource.Get(),
		nullptr,
		_renderTargetView.getCPUHandle()
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
		_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pResource.Get(),
			nullptr,
			_shaderResourceView.getCPUHandle()
		);
	}
}

RenderTargetBuffer::RenderTargetBuffer(std::weak_ptr<Device> 
	pDevice, WRL::ComPtr<ID3D12Resource> pResrouce, 
	D3D12_RESOURCE_STATES state)
: _pResource(pResrouce) 
{
	createViews(pDevice);
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}

RenderTargetBuffer::RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
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

	createViews(pDevice);

	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COMMON);
}

}