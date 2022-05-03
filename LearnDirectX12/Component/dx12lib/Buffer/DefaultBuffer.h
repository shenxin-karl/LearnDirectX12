#pragma once
#include "dx12libStd.h"
#include "IResource.h"

namespace dx12lib {

class DefaultBuffer : public IBufferResource {
public:
	DefaultBuffer() = default;
	DefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, const void *pData, size_t sizeInByte);
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