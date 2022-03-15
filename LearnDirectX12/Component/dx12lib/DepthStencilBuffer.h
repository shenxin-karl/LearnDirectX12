#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class DepthStencilBuffer : IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const;
	~DepthStencilBuffer() override;
protected:
	DepthStencilBuffer(std::weak_ptr<Device> pDevice, 
		uint32 width, 
		uint32 height,
		D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN
	);
	DepthStencilBuffer(std::weak_ptr<Device> pDevice, WRL::ComPtr<ID3D12Resource> pResource);
	void createViews(std::weak_ptr<Device> pDevice);
private:
	DescriptorAllocation _depthStencilView;
	DescriptorAllocation _shaderResourceView;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}