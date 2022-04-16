#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class ShaderResourceBuffer : public IShaderSourceResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	bool isShaderSample() const override;
	~ShaderResourceBuffer() override;
protected:
	ShaderResourceBuffer(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state,
		const D3D12_SHADER_RESOURCE_VIEW_DESC *pSrvDesc = nullptr
	);
	void createViews(std::weak_ptr<Device> pDevice, const D3D12_SHADER_RESOURCE_VIEW_DESC *pSrvDesc);
private:
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	DescriptorAllocation        _shaderResourceView;
};

}