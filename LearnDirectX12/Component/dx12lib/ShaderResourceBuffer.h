#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class ShaderResourceBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const;
	~ShaderResourceBuffer();
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

class Texture2D : public IShaderSourceResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	~Texture2D() override = default;
protected:
	Texture2D(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
	void initSrvDesc();
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable D3D12_SHADER_RESOURCE_VIEW_DESC _srvDesc;
};

}