#pragma once
#include "dx12libStd.h"
#include "IResource.h"

namespace dx12lib {

class DefaultBuffer : public IResource {
public:
	DefaultBuffer() = default;
	DefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, const void *pData, size_t sizeInByte);
	DefaultBuffer(const DefaultBuffer &) = delete;
	DefaultBuffer(DefaultBuffer &&other) noexcept;
	DefaultBuffer &operator=(DefaultBuffer &&other) noexcept;
	~DefaultBuffer();
	D3D12_GPU_VIRTUAL_ADDRESS getAddress() const;
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	friend void swap(DefaultBuffer &lhs, DefaultBuffer &rhs) noexcept;
private:
	WRL::ComPtr<ID3D12Resource>  _pDefaultBuffer;
	WRL::ComPtr<ID3D12Resource>  _pUploader;
};

}