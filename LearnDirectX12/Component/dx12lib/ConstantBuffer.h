#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class UploadBuffer;

class ConstantBuffer : public IConstantBuffer {
protected:
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, uint32 sizeInByte);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset = 0) override;
	BYTE *getMappedPtr();
	const BYTE *getMappedPtr() const;
	uint32 getConstantBufferSize() const noexcept override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
private:
	uint32 _bufferSize;
	DescriptorAllocation _CBV;
	std::unique_ptr<UploadBuffer> _pGPUBuffer;
};

}
