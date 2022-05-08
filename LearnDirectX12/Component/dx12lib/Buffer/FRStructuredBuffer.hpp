#pragma once
#include <dx12lib/Device/Device.h>
#include <dx12lib/Buffer/UploadBuffer.h>
#include <dx12lib/Descriptor/DescriptorAllocation.h>
#include <dx12lib/Resource/ResourceView.hpp>
#include <dx12lib/Resource/IResource.h>
#include <span>


namespace dx12lib {

// frame resource structured buffer template
template<>
class FRStructuredBuffer<RawData> : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementCount() const override;
	size_t getElementStride() const override;
	BYTE *getMappedPtr() override;
	const BYTE *getMappedPtr() const override;
	void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	StructuredBufferView getSRV() const override;

	template<typename T>
	std::span<T> visit() {
		assert(sizeof(T) == getElementStride());
		size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
		size_t numElements = getElementCount();
		T *ptr = reinterpret_cast<T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
		return std::span<T>(ptr, numElements);
	}

	template<typename T>
	std::span<const T> visit() const {
		assert(sizeof(T) == getElementStride());
		size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
		size_t numElements = getElementCount();
		const T *ptr = reinterpret_cast<const T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
		return std::span<const T>(ptr, numElements);
	}
private:
	size_t _elementStride;
	DescriptorAllocation _descriptor;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};


/********************************************************************************************************/

// frame resource structured buffer template
template<typename T>
class FRStructuredBuffer : public IStructuredBuffer {
protected:
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, const T *pData, size_t numElements);
	FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementCount() const override;
	size_t getElementStride() const override;
	BYTE *getMappedPtr() override;
	const BYTE *getMappedPtr() const override;
	void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	StructuredBufferView getSRV() const override;
	std::span<T> visit();
	std::span<const T> visit() const;
private:
	DescriptorAllocation _descriptor;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

template <typename T>
FRStructuredBuffer<T>::FRStructuredBuffer(std::weak_ptr<Device> pDevice, const T *pData, size_t numElements) {
	size_t stride = sizeof(T);
	size_t sizeInByte = numElements * stride;
	auto pSharedDevice = pDevice.lock();
	_descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
	desc.Buffer.NumElements = static_cast<UINT>(numElements);
	desc.Buffer.StructureByteStride = static_cast<UINT>(stride);
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	_descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		if (pData != nullptr)
			_pUploadBuffer->copyData(i, pData, sizeInByte, 0);

		desc.Buffer.FirstElement = i * numElements;
		pSharedDevice->getD3DDevice()->CreateShaderResourceView(
			_pUploadBuffer->getD3DResource().Get(),
			&desc,
			_descriptor.getCPUHandle(i)
		);
	}
}

template <typename T>
FRStructuredBuffer<T>::FRStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements) 
: FRStructuredBuffer(pDevice, nullptr, numElements)
{
}

template <typename T>
WRL::ComPtr<ID3D12Resource> FRStructuredBuffer<T>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

template <typename T>
size_t FRStructuredBuffer<T>::getBufferSize() const {
	return _pUploadBuffer->getElementByteSize();
}

template <typename T>
size_t FRStructuredBuffer<T>::getElementCount() const {
	return getBufferSize() / sizeof(T);
}

template <typename T>
size_t FRStructuredBuffer<T>::getElementStride() const {
	return sizeof(T);
}

template <typename T>
BYTE *FRStructuredBuffer<T>::getMappedPtr() {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	return _pUploadBuffer->getMappedDataByIndex(frameIndex);
}

template <typename T>
const BYTE *FRStructuredBuffer<T>::getMappedPtr() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	return _pUploadBuffer->getMappedDataByIndex(frameIndex);
}

template <typename T>
void FRStructuredBuffer<T>::updateBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	assert((sizeInByte + offset) <= getElementStride());
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	_pUploadBuffer->copyData(frameIndex, pData, sizeInByte, offset);
}

template <typename T>
StructuredBufferView FRStructuredBuffer<T>::getSRV() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	return StructuredBufferView(_descriptor, this, frameIndex);
}

template <typename T>
std::span<T> FRStructuredBuffer<T>::visit() {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	size_t numElements = getElementCount();
	T *ptr = reinterpret_cast<T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
	return std::span<T>(ptr, numElements);
}

template <typename T>
std::span<const T> FRStructuredBuffer<T>::visit() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	size_t numElements = getElementCount();
	const T *ptr = reinterpret_cast<const T *>(_pUploadBuffer->getMappedDataByIndex(frameIndex));
	return std::span<const T>(ptr, numElements);
}


}
