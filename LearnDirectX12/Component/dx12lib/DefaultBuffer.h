#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class DefaultBuffer {
public:
	DefaultBuffer() = default;
	DefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, const void *pData, uint32 sizeInByte);
	DefaultBuffer(const DefaultBuffer &) = delete;
	DefaultBuffer(DefaultBuffer &&other) noexcept;
	DefaultBuffer &operator=(DefaultBuffer &&other) noexcept;
	~DefaultBuffer() = default;
	D3D12_GPU_VIRTUAL_ADDRESS getAddress() const;
	ID3D12Resource *getResource() const;
	friend void swap(DefaultBuffer &lhs, DefaultBuffer &rhs) noexcept;
private:
	WRL::ComPtr<ID3D12Resource>  _pDefaultBuffer;
	WRL::ComPtr<ID3D12Resource>  _pUploader;
};

}