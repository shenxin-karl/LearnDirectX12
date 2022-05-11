#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/IResource.h>

namespace dx12lib {

class DefaultBuffer : public IBufferResource {
public:
	DefaultBuffer(ID3D12Device *pDevice, 
		ID3D12GraphicsCommandList *pCmdList, 
		const void *pData, 
		size_t sizeInByte,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_GENERIC_READ
	);
	~DefaultBuffer() override;
	D3D12_GPU_VIRTUAL_ADDRESS getAddress() const;
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	BufferType getBufferType() const override;
	size_t getBufferSize() const override;
private:
	WRL::ComPtr<ID3D12Resource>  _pDefaultResource;
	WRL::ComPtr<ID3D12Resource>  _pUploaderResource;
};

}