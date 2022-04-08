#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class UploadBuffer;

class ConstantBuffer : public IConstantBuffer {
protected:
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, std::size_t sizeInByte);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	BYTE *getMappedPtr();
	const BYTE *getMappedPtr() const;
	size_t getConstantBufferSize() const noexcept override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
private:
	DescriptorAllocation _CBV;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}
