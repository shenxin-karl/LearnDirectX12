#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {
	
class UnorderedAccessBuffer : public IShaderSourceResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getUnorderedAccessView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	bool isShaderSample() const override;
	std::size_t getBufferSize() const;
	~UnorderedAccessBuffer();
protected:
	void createViews(std::weak_ptr<Device> pDevice, DXGI_FORMAT format);
	UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, std::size_t width, std::size_t height, DXGI_FORMAT format);
	UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
private:
	DescriptorAllocation _shaderResourceView;
	DescriptorAllocation _unorderedAccessView;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}