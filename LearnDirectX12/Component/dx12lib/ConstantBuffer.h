#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class UploadBuffer;
class ConstantBuffer {
public:
	ConstantBuffer();
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, uint32 sizeInByte);
	ConstantBuffer(const ConstantBuffer &) = delete;
	ConstantBuffer(ConstantBuffer &&other) noexcept;
	ConstantBuffer &operator=(ConstantBuffer &&other) noexcept;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset);
	uint32 getConstantBufferSize() const noexcept;
	uint32 getConstantAlignedBufferSize() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const;
	bool isEmpty() const noexcept;
	friend void swap(ConstantBuffer &lhs, ConstantBuffer &rhs) noexcept;
private:
	uint32                        _bufferSize;
	DescriptorAllocation          _CBV;
	std::unique_ptr<UploadBuffer> _pGPUBuffer;
};

}