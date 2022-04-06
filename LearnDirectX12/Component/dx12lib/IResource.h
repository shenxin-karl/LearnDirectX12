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
	virtual bool isMapped() const;
	virtual DXGI_FORMAT getFormat() const;
	ResourceType getResourceType() const {  return _resourceType; }
protected:
	IResource() = default;
	IResource(const IResource &) = delete;
	IResource &operator=(const IResource &) = delete;
	ResourceType _resourceType = ResourceType::Unknown;
};

class IShaderSourceResource : public IResource {
public:
	virtual D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const = 0;
	virtual bool isShaderSample() const = 0;
};


}