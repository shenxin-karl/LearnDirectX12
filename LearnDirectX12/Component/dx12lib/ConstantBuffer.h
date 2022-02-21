#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

class UploadBuffer;

struct ConstantBufferDesc {
	std::weak_ptr<Device>  pDevice;
	std::atomic_uint32_t  &frameIndex;
	uint32                 frameCount;
	uint32                 sizeInByte;
	const void            *pData = nullptr;
};

class ConstantBuffer {
protected:
	ConstantBuffer(const ConstantBufferDesc &desc);
public:
	ConstantBuffer(const ConstantBuffer &) = delete;
	ConstantBuffer &operator=(const ConstantBuffer &) = delete;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset = 0);
	BYTE *getMappedPtr();
	BYTE *getMappedPtr() const;
	uint32 getConstantBufferSize() const noexcept;
	uint32 getConstantAlignedBufferSize() const noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const;
	uint32 getFrameIndex() const;
private:
	uint32                            _bufferSize;
	const std::atomic_uint32_t        &_frameIndex;
	std::unique_ptr<UploadBuffer>     _pGPUBuffer;
	std::vector<DescriptorAllocation> _CBV;
};

}