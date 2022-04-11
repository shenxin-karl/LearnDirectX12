#pragma once
#include "Device.h"
#include "UploadBuffer.h"
#include "DescriptorAllocation.h"
#include "UploadBuffer.h"
#include <span>

namespace dx12lib {

// frame resource structured buffer template
template<>
class FRStructuredBuffer<RawData> : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getStructuredBufferSize() const override;
	size_t getElementCount() const override;
	size_t getStride() const override;

	template<typename T>
	std::span<T> visit() {
		assert(sizeof(T) == getStride());
		size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
		size_t numElements = getElementCount();
		T *ptr = reinterpret_cast<T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
		return std::span<T>(ptr, numElements);
	}

	template<typename T>
	std::span<const T> visit() const {
		assert(sizeof(T) == getStride());
		size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
		size_t numElements = getElementCount();
		const T *ptr = reinterpret_cast<const T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
		return std::span<const T>(ptr, numElements);
	}
private:
	DescriptorAllocation _structuredBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};


/********************************************************************************************************/

// frame resource structured buffer template
template<typename T>
class FRStructuredBuffer : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, const T *pData);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getShaderResourceView() const override;
	void updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getStructuredBufferSize() const override;
	size_t getElementCount() const override;
	size_t getStride() const override;
	std::span<T> visit();
	std::span<const T> visit() const;
private:
	DescriptorAllocation _structuredBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

template <typename T>
FRStructuredBuffer<T>::FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, const T *pData) {
	size_t stride = sizeof(T);
	size_t sizeInByte = numElements * stride;
	auto pSharedDevice = pDevice.lock();
	_structuredBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		kFrameResourceCount,
		sizeInByte,
		false
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = _pUploadBuffer->getD3DResource()->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.NumElements = numElements;
	desc.Buffer.StructureByteStride = stride;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	_structuredBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		if (pData != nullptr)
			_pUploadBuffer->copyData(i, pData, sizeInByte, 0);

		desc.Buffer.FirstElement = i * numElements;
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pUploadBuffer->getD3DResource().Get(),
			&desc,
			_structuredBufferView.getCPUHandle(i)
		);
	}
}

template <typename T>
WRL::ComPtr<ID3D12Resource> FRStructuredBuffer<T>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

template <typename T>
D3D12_CPU_DESCRIPTOR_HANDLE FRStructuredBuffer<T>::getShaderResourceView() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	return _structuredBufferView.getCPUHandle(frameIndex);
}

template <typename T>
void FRStructuredBuffer<T>::updateStructuredBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	_pUploadBuffer->copyData(frameIndex, pData, sizeInByte, offset);
}

template <typename T>
size_t FRStructuredBuffer<T>::getStructuredBufferSize() const {
	return _pUploadBuffer->getWidth();
}

template <typename T>
size_t FRStructuredBuffer<T>::getElementCount() const {
	return _pUploadBuffer->getElementCount();
}

template <typename T>
size_t FRStructuredBuffer<T>::getStride() const {
	return _pUploadBuffer->getElementByteSize();
}

template <typename T>
std::span<T> FRStructuredBuffer<T>::visit() {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	T *ptr = reinterpret_cast<T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
	return std::span<T>(ptr, getElementCount());
}

template <typename T>
std::span<const T> FRStructuredBuffer<T>::visit() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	const T *ptr = reinterpret_cast<const T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
	return std::span<const T>(ptr, getElementCount());
}

}
