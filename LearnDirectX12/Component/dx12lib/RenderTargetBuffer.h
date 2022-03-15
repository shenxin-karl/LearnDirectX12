#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class RenderTargetBuffer : public IResource {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	uint64 getWidth() const override;
	uint64 getHeight() const override;
	uint64 getDepth() const override;
	~RenderTargetBuffer() override;
	D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const;
protected:
	//RenderTargetBuffer()
private:
	DescriptorAllocation _renderTargetView;
	DescriptorAllocation _shaderResourceView;
	WRL::ComPtr<ID3D12Resource> _pResource;
};

}