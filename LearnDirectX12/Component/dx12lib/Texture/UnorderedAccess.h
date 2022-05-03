#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {
	
class UnorderedAccess2D : public IUnorderedAccess2D {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	UnorderedAccessView getUAV(size_t mipSlice = 0) const override;
protected:
	UnorderedAccess2D(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state
	);
	UnorderedAccess2D(std::weak_ptr<Device> pDevice,
		size_t width,
		size_t height,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable ViewManager<UnorderedAccessView> _uavMgr;
};

class UnorderedAccess2DArray : public IUnorderedAccess2DArray {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(size_t planeSlice, size_t mipSlice = 0) const override;
	UnorderedAccessView getUAV(size_t planeSlice, size_t mipSlice = 0) const override;
protected:
	UnorderedAccess2DArray(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		D3D12_RESOURCE_STATES state
	);
	UnorderedAccess2DArray(std::weak_ptr<Device> pDevice,
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
	mutable std::map<size_t, ViewManager<ShaderResourceView>> _planeSrvMgr;
	mutable std::map<size_t, ViewManager<UnorderedAccessView>> _planeUavMgr;
};

class UnorderedAccessCube : public IUnorderedAccessCube {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	ShaderResourceView getSRV(size_t mipSlice = 0) const override;
	ShaderResourceView getSRV(CubeFace face, size_t mipSlice = 0) const override;
	UnorderedAccessView getUAV(CubeFace face, size_t mipSlice = 0) const override;
protected:
	UnorderedAccessCube(std::weak_ptr<Device> pDevice,
		WRL::ComPtr<ID3D12Resource> pResource,
		D3D12_RESOURCE_STATES state
	);
	UnorderedAccessCube(std::weak_ptr<Device> pDevice,
		size_t width,
		size_t height,
		const D3D12_CLEAR_VALUE *pClearValue = nullptr,
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN
	);
private:
	std::weak_ptr<Device> _pDevice;
	WRL::ComPtr<ID3D12Resource> _pResource;
	mutable ViewManager<ShaderResourceView> _srvMgr;
	mutable std::map<CubeFace, ViewManager<ShaderResourceView>> _cubeSrvMgr;
	mutable std::map<CubeFace, ViewManager<UnorderedAccessView>> _cubeUavMgr;
};

}