#include "ShaderResourceBuffer.h"
#include "ResourceStateTracker.h"
#include "Device.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> ShaderResourceBuffer::getD3DResource() const {
	return _pResource;
}

ShaderResourceView ShaderResourceBuffer::getShaderResourceView(size_t mipSlice) const {
	return ShaderResourceView();
}

ShaderResourceBuffer::~ShaderResourceBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

ShaderResourceBuffer::ShaderResourceBuffer(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource, 
	WRL::ComPtr<ID3D12Resource> pUploader,
	D3D12_RESOURCE_STATES state,
	const D3D12_SHADER_RESOURCE_VIEW_DESC *pSrvDesc)
{
	assert(pResource != nullptr);
	_pResource = pResource;
	_pUploader = pUploader;
	createViews(pDevice, pSrvDesc);
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}


void ShaderResourceBuffer::createViews(std::weak_ptr<Device> pDevice, const D3D12_SHADER_RESOURCE_VIEW_DESC *pSrvDesc) {
	auto pSharedDevice = pDevice.lock();
	auto desc = _pResource->GetDesc();
	D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
	formatSupport.Format = desc.Format;
	ThrowIfFailed(pSharedDevice->getD3DDevice()->CheckFeatureSupport(
		D3D12_FEATURE_FORMAT_SUPPORT,
		&formatSupport,
		sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)
	));

	if (!(formatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE))
		throw std::runtime_error("invalid shader resource buffer");

	_shaderResourceView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		pSrvDesc,
		_shaderResourceView.getCPUHandle()
	);
}

WRL::ComPtr<ID3D12Resource> Texture2D::getD3DResource() const {
	return _pResource;
}

ShaderResourceView Texture2D::getShaderResourceView(size_t mipSlice) const {
	assert(mipSlice < _pResource->GetDesc().MipLevels);
	auto pSharedDevice = _pDevice.lock();
	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_srvDesc.Texture2D.MostDetailedMip = mipSlice;
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pResource.Get(),
		&_srvDesc,
		descriptor.getCPUHandle()
	);
	return ShaderResourceView(descriptor);
}

Texture2D::Texture2D(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource,
	WRL::ComPtr<ID3D12Resource> pUploader, D3D12_RESOURCE_STATES state)
: _pDevice(pDevice), _pResource(pResource), _pUploader(pUploader), _srvDesc({})
{
	initSrvDesc();
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}

void Texture2D::initSrvDesc() {
	_srvDesc.Format = _pResource.Get()->GetDesc().Format;
	_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	_srvDesc.Texture2D.MipLevels = _pResource.Get()->GetDesc().MipLevels;
	_srvDesc.Texture2D.PlaneSlice = 0;
	_srvDesc.Texture2D.MostDetailedMip = 0;
	_srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
}

}
