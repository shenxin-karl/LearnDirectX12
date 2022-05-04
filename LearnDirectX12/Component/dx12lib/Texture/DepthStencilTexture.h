#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Descriptor/DescriptorAllocation.h>

namespace dx12lib {

class DepthStencil2D : public IDepthStencil2D {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	DepthStencilView getDSV() const override;
	~DepthStencil2D() override;
protected:
	DepthStencil2D(std::weak_ptr<Device> pDevice, 
		size_t width, 
		size_t height,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_UNKNOWN
	);
	DepthStencil2D(std::weak_ptr<Device> pDevice, 
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