#pragma once
#include "dx12libStd.h"
#include <d3d12.h>
#include <wrl.h>

namespace dx12lib {

class IResource {
public:
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const = 0;
	virtual ~IResource() = default;
};

class Resource : public IResource {
protected:
	Resource(ID3D12Device *pDevice, const D3D12_RESOURCE_DESC &desc, const D3D12_CLEAR_VALUE *pClearValue = nullptr);
	Resource(ID3D12Device *pDevice, WRL::ComPtr<ID3D12Resource> pResource);
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void checkFeatureSupport(ID3D12Device *pDevice);
	bool checkFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const;
protected:
	WRL::ComPtr<ID3D12Resource>       _pResource;
	D3D12_FEATURE_DATA_FORMAT_SUPPORT _formatSupport;
};

}