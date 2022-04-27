#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class RenderTarget2D : public IShaderResourceBuffer, public IRenderTargetBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~RenderTarget2D() override;
	RenderTargetView getRenderTargetView(size_t mipSlice = 0) const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	ShaderResourceType getShaderResourceType() const override;
	ResourceType getResourceType() const override;
protected:
	RenderTarget2D(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
	RenderTarget2D(std::weak_ptr<Device> pDevice, 
		uint32 width, 
		uint32 height, 
		D3D12_CLEAR_VALUE *pClearValue = nullptr, 
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<RenderTargetView> _rtvMgr;
	mutable ViewManager<ShaderResourceView> _srvMgr;
};

}