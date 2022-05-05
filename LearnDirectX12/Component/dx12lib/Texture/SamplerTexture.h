#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Descriptor/DescriptorAllocation.h>

namespace dx12lib {

class SamplerTexture2D : public IShaderResource2D {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	~SamplerTexture2D() override = default;
protected:
	SamplerTexture2D(std::weak_ptr<Device> pDevice,
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

class SamplerTexture2DArray : public IShaderResource2DArray {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(size_t planeSlice, size_t mipSlice = 0) const override;
	~SamplerTexture2DArray() override = default;
protected:
	SamplerTexture2DArray(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable std::map<size_t, ViewManager<ShaderResourceView>> _planeSrvMgr;
};

class SamplerTextureCube : public IShaderResourceCube {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(CubeFace face, size_t mipSlice = 0) const override;
	~SamplerTextureCube() override = default;
protected:
	SamplerTextureCube(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		WRL::ComPtr<ID3D12Resource> pUploader,
		D3D12_RESOURCE_STATES state
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	WRL::ComPtr<ID3D12Resource> _pUploader;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable std::map<CubeFace, ViewManager<ShaderResourceView>> _faceMapSrvMgr;
};

}