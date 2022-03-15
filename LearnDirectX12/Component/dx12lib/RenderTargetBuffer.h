#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class RenderTargetBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~RenderTargetBuffer() override;
	D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const;
protected:
	void createViews(std::weak_ptr<Device> pDevice);
	RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResrouce, 
		D3D12_RESOURCE_STATES state
	);
	RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
		uint32 width, 
		uint32 height, 
		D3D12_CLEAR_VALUE *pClearValue = nullptr, 
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	DescriptorAllocation _renderTargetView;
	DescriptorAllocation _shaderResourceView;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}