#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class UploadBuffer;
class ConstantBuffer {
	ConstantBuffer();
	ConstantBuffer(ID3D12Device *pDevice, const void *pData, uint32 sizeInByte);
	ConstantBuffer(const ConstantBuffer &) = delete;
	ConstantBuffer(ConstantBuffer &&other) noexcept;
	ConstantBuffer &operator=(ConstantBuffer &&other) noexcept;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset);
	uint32 getConstantBufferSize() const noexcept;
	uint32 getConstantAlignedBufferSize() const noexcept;
	bool isEmpty() const noexcept;
	friend void swap(ConstantBuffer &lhs, ConstantBuffer &rhs) noexcept;
private:
	std::unique_ptr<UploadBuffer>  _pGPUBuffer;
	uint32 _bufferSize;
};

}