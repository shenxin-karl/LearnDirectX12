#pragma once
#include "UploadBuffer.h"
#include "DescriptorAllocation.h"
#include <span>

namespace dx12lib {

// frame resource structured buffer template
template<>
class FRStructuredBuffer<RawData> : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void *pData);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getStructuredBufferSize() const override;

	template<typename T>
	std::span<T> visitor();

	template<typename T>
	std::span<const T> visitor() const;
private:
	DescriptorAllocation _structedBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};


/********************************************************************************************************/

// frame resource structured buffer template
template<typename T>
class FRStructuredBuffer : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void *pData);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getStructuredBufferSize() const override;
	std::span<T> visitor();
	std::span<const T> visitor() const;
private:
	DescriptorAllocation _structedBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}
