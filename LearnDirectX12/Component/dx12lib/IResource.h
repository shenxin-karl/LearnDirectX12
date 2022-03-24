#pragma once
#include "dx12libStd.h"
#include <d3d12.h>
#include <wrl.h>

namespace dx12lib {

class IResource {
public:
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const = 0;
	virtual ~IResource() = default;
	virtual uint64 getWidth() const;
	virtual uint64 getHeight() const;
	virtual uint64 getDepth() const;
	virtual DXGI_FORMAT getFormat() const;
	ResourceType getResourceType() const {  return _resourceType; }
protected:
	IResource() = default;
	IResource(const IResource &) = delete;
	IResource &operator=(const IResource &) = delete;
	ResourceType _resourceType = ResourceType::Unknow;
};

class IShaderSourceResource : public IResource {
public:
	virtual D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const = 0;
	virtual bool isShaderSample() const = 0;
};

class Resource : public IResource {
protected:
	Resource(ID3D12Device *pDevice, 
		const D3D12_RESOURCE_DESC &desc, 
		const D3D12_CLEAR_VALUE *pClearValue = nullptr
	);
	Resource(ID3D12Device *pDevice, 
		WRL::ComPtr<ID3D12Resource> pResource, 
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON
	);
	void setD3DResource(WRL::ComPtr<ID3D12Resource> pResource, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON);
	void checkFeatureSupport(ID3D12Device *pDevice);
public:
	bool checkFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	virtual ~Resource() override;
private:
	WRL::ComPtr<ID3D12Resource>       _pResource;
	D3D12_FEATURE_DATA_FORMAT_SUPPORT _formatSupport;
};

}