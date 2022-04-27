#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class DepthStencilBuffer : public IDepthStencilBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	DepthStencilView getDepthStencilView() const override;
	ShaderResourceView getShaderResourceView() const override;
	ResourceType getResourceType() const override;
	~DepthStencilBuffer() override;
protected:
	DepthStencilBuffer(std::weak_ptr<Device> pDevice, 
		uint32 width, 
		uint32 height,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN
	);
	DepthStencilBuffer(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource,
		D3D12_RESOURCE_STATES state
	);
	void createViews(std::weak_ptr<Device> pDevice);
private:
	DepthStencilView _depthStencilView;
	ShaderResourceView _shaderResourceView;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}