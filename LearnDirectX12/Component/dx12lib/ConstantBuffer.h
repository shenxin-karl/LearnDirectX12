#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class UploadBuffer;

class ConstantBuffer {
protected:
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, std::size_t sizeInByte);
public:
	ConstantBuffer(const ConstantBuffer &) = delete;
	ConstantBuffer &operator=(const ConstantBuffer &) = delete;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset = 0);
	BYTE *getMappedPtr();
	const BYTE *getMappedPtr() const;
	uint32 getConstantBufferSize() const noexcept;
	uint32 getConstantAlignedBufferSize() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const;
private:
	std::size_t _bufferSize;
	DescriptorAllocation _CBV;
	std::unique_ptr<UploadBuffer> _pGPUBuffer;
};

}