#pragma once
#include "dx12libCommon.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class Texture {
public:
	void resize(uint32 width, uint32 height, uint32 depthOrArraySize = 1);
	D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const;
	//D3D12_CPU_DESCRIPTOR_HANDLE getUnorderedAccessView() const;
	bool hasAlpha() const;
	size_t bitsPerPixel() const;
private:
	friend class Device;
	Texture(std::weak_ptr<Device> pDevice, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE *pClearValue = nullptr);
	Texture(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource, const D3D12_CLEAR_VALUE *pClearValue = nullptr);
	void checkFeatureSupport();
	bool checkFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
private:
	DescriptorAllocation                _renderTargetView;
	DescriptorAllocation                _depthStencilView;
	DescriptorAllocation                _shaderResourceView;
	std::weak_ptr<Device>               _pDevice;
	WRL::ComPtr<ID3D12Resource>         _pResource;
	std::unique_ptr<D3D12_CLEAR_VALUE>  _pClearValue;
	D3D12_FEATURE_DATA_FORMAT_SUPPORT   _formatSupport;
	//DescriptorAllocation _unorderedAccessView;
};

}