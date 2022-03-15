#include "DepthStencilBuffer.h"
#include "ResourceStateTracker.h"
#include "Device.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> DepthStencilBuffer::getD3DResource() const {
	return _pResource;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilBuffer::getDepthStencilView() const {
	return _depthStencilView.getCPUHandle();
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilBuffer::getShaderResourceView() const {
	return _shaderResourceView.getCPUHandle();
}

DepthStencilBuffer::~DepthStencilBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

DepthStencilBuffer::DepthStencilBuffer(std::weak_ptr<Device> pDevice, 
	uint32 width, 
	uint32 height, 
	D3D12_CLEAR_VALUE *pClearValue = nullptr,
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN)
{
	auto pSharedDevice = pDevice.lock();
	if (depthStencilFormat == DXGI_FORMAT_UNKNOWN)
		depthStencilFormat = pSharedDevice->getDesc().depthStencilFormat;

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
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

	_resourceType = ResourceType::DepthStencilBuffer | ResourceType::ShaderResourceBuffer;
	createViews(pDevice);
}

DepthStencilBuffer::DepthStencilBuffer(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource) {
	_resourceType = ResourceType::DepthStencilBuffer | ResourceType::ShaderResourceBuffer;
	_pResource = pResource;
	createViews(pDevice);
}

void DepthStencilBuffer::createViews(std::weak_ptr<Device> pDevice) {
	auto pSharedDevice = pDevice.lock();
	_depthStencilView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	pSharedDevice->getD3DDevice()->CreateDepthStencilView(
		_pResource.Get(),
		nullptr,
		_depthStencilView.getCPUHandle()
	);

	_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		nullptr,
		_shaderResourceView.getCPUHandle()
	);
}

}
