#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class ShaderResourceBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const;
	ShaderResourceType getShaderResourceType() const;
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

class Texture2D : public IShaderResourceBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	ShaderResourceType getShaderResourceType() const override;
	~Texture2D() override = default;
protected:
	Texture2D(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable ViewManager<ShaderResourceView> _srvMgr;
};

class TextureCube : public IShaderResourceBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	ShaderResourceView getShaderResourceView(CubeFace face, size_t mipSlice) const;
	ShaderResourceType getShaderResourceType() const override;
	~TextureCube() override = default;
protected:
	TextureCube(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable ViewManager<ShaderResourceView> _faceSrvMgr;
};


class TextureArray : public IShaderResourceBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getShaderResourceView(size_t mipSlice = 0) const override;
	ShaderResourceType getShaderResourceType() const override;
	~TextureArray() override = default;
protected:
	TextureArray(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable ViewManager<ShaderResourceView> _srvMgr;
};

}