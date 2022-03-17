#include "ShaderResourceBuffer.h"
#include "ResourceStateTracker.h"
#include "Device.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> ShaderResourceBuffer::getD3DResource() const {
	return _pResource;
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceBuffer::getShaderResourceView() const {
	assert(isShaderSample());
	return _shaderResourceView.getCPUHandle();
}

bool ShaderResourceBuffer::isShaderSample() const {
	return _shaderResourceView.isValid();
}

ShaderResourceBuffer::~ShaderResourceBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

ShaderResourceBuffer::ShaderResourceBuffer(std::weak_ptr<Device> pDevice, 
	WRL::ComPtr<ID3D12Resource> pResource, 
	WRL::ComPtr<ID3D12Resource> pUploader,
	D3D12_RESOURCE_STATES state)
{
	assert(pResource != nullptr);
	_pResource = pResource;
	_pUploader = pUploader;
	createViews(pDevice);
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), state);
}


void ShaderResourceBuffer::createViews(std::weak_ptr<Device> pDevice) {
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
		nullptr,
		_shaderResourceView.getCPUHandle()
	);
}

}