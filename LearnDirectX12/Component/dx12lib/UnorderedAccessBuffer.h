#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {
	
class UnorderedAccessBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	UnorderedAccessView getUnorderedAccessView(size_t mipSlice = 0) const;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const;
	std::size_t getBufferSize() const;
	~UnorderedAccessBuffer() override;
protected:
	UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, std::size_t width, std::size_t height, DXGI_FORMAT format);
	UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
	void initViewDesc(DXGI_FORMAT format);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable D3D12_UNORDERED_ACCESS_VIEW_DESC _uavDesc;
	mutable D3D12_SHADER_RESOURCE_VIEW_DESC _srvDesc;
};

}