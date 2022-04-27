#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class RenderTargetBuffer : public IShaderSourceResource, public IRenderTargetResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~RenderTargetBuffer() override;
	RenderTargetView getRenderTargetView(size_t mipSlice = 0) const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	ResourceType getResourceType() const override;
protected:
	RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
	RenderTargetBuffer(std::weak_ptr<Device> pDevice, 
		uint32 width, 
		uint32 height, 
		D3D12_CLEAR_VALUE *pClearValue = nullptr, 
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
	void initViewDesc(DXGI_FORMAT format);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable D3D12_RENDER_TARGET_VIEW_DESC _rtvDesc;
	mutable	D3D12_SHADER_RESOURCE_VIEW_DESC _srvDesc;
};

}