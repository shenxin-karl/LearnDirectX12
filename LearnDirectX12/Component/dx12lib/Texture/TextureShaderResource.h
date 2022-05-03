#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class Texture2D : public IShaderResource2D {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
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

class Texture2DArray : public IShaderResource2DArray {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(size_t planeSlice, size_t mipSlice = 0) const override;
	~Texture2DArray() override = default;
protected:
	Texture2DArray(std::weak_ptr<Device> pDevice,
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

class TextureCube : public IShaderResourceCube {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(CubeFace face, size_t mipSlice = 0) const override;
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
	mutable std::map<CubeFace, ViewManager<ShaderResourceView>> _faceMapSrvMgr;
};

}