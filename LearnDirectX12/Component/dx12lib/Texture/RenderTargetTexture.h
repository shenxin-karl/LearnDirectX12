#pragma once
#include <dx12lib/Resource/IResource.h>

namespace dx12lib {

class RenderTarget2D : public IRenderTarget2D {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	RenderTargetView getRTV(size_t mipSlice = 0) const override;
protected:
	RenderTarget2D(std::weak_ptr<Device> pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
	RenderTarget2D(std::weak_ptr<Device> pDevice,
		size_t width, 
		size_t height,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<RenderTargetView> _rtvMgr;
	mutable ViewManager<ShaderResourceView> _srvMgr;
};

class RenderTarget2DArray : public IRenderTarget2DArray {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getPlaneSRV(size_t planeSlice, size_t mipSlice = 0) const override;
	RenderTargetView getPlaneRTV(size_t planeSlice, size_t mipSlice = 0) const override;
protected:
	RenderTarget2DArray(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		D3D12_RESOURCE_STATES state
	);
	RenderTarget2DArray(std::weak_ptr<Device> pDevice,
		size_t width,
		size_t height,
		size_t planeSlice,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable std::map<size_t, ViewManager<RenderTargetView>> _planeRtvMgr;
	mutable std::map<size_t, ViewManager<ShaderResourceView>> _planeSrvMgr;
};

class RenderTargetCube : public IRenderTargetCube {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getFaceSRV(CubeFace face, size_t mipSlice) const override;
	RenderTargetView getFaceRTV(CubeFace face, size_t mipSlice = 0) const override;
protected:
	RenderTargetCube(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		D3D12_RESOURCE_STATES state
	);
	RenderTargetCube(std::weak_ptr<Device> pDevice,
		uint32 width,
		uint32 height,
		D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable std::map<CubeFace, ViewManager<RenderTargetView>> _cubeRtvMgr;
	mutable std::map<CubeFace, ViewManager<ShaderResourceView>> _cubeSrvMgr;
};


}