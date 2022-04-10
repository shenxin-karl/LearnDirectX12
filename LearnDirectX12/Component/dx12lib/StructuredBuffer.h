#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class StructuredBuffer : public IStructuredBuffer {
public:
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	size_t getStructuredBufferSize() const override;
	size_t getElementCount() const override;
	size_t getStride() const override;
	~StructuredBuffer() override;
protected:
	StructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride);
private:
	DescriptorAllocation _structuredBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}