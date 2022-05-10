#pragma once
#include <dx12lib/Device/Device.h>
#include <dx12lib/Buffer/UploadBuffer.h>
#include <dx12lib/Resource/ResourceView.hpp>
#include <dx12lib/Resource/IResource.h>
#include <span>

namespace dx12lib {

class ConsumeStructuredBuffer : public IConsumeStructuredBuffer {
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementCount() const override;
	size_t getElementStride() const override;
	UnorderedAccessView getUAV() const override;
	void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;

	template<typename T>
	std::span<T> visit() {
		assert(sizeof(T) == getElementStride());
		return std::span<T>(_pUploadBuffer->getMappedDataByIndex(0), getElementCount());
	}
protected:
	ConsumeStructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride);
	ConsumeStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, size_t stride);
private:
	size_t _elementStride;
	UnorderedAccessView _uav;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}
