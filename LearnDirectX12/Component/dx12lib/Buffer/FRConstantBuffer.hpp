#pragma once
#include <dx12lib/Device/Device.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Buffer/UploadBuffer.h>
#include <dx12lib/Buffer/CBufferVisitor.hpp>
#include <dx12lib/Descriptor/DescriptorAllocation.h>

namespace dx12lib {

template<>
class FRConstantBuffer<RawData> : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void *pData);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementStride() const override;
	BYTE *getMappedPtr() override;
	const BYTE *getMappedPtr() const override;
	void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	ConstantBufferView getCBV() const override;

	template<typename T>
	T *map() {
		assert(sizeof(T) <= getElementStride());
		return reinterpret_cast<T *>(_pObject.get());
	}

	template<typename T>
	const T *cmap() const {
		assert(sizeof(T) <= getElementStride());
		return reinterpret_cast<const T *>(_pObject.get());
	}

	template<typename T>
	CBufferVisitor<T> visit() {
		assert(sizeof(T) <= getElementStride());
		return CBufferVisitor<T>(map<T>());
	}

	template<typename T>
	CBufferVisitor<const T> visit() const {
		assert(sizeof(T) <= getElementStride());
		return CBufferVisitor<T>(cmap<T>());
	}
private:
	DescriptorAllocation _descriptor;
	std::unique_ptr<BYTE[]> _pObject;
	mutable std::unique_ptr<UploadBuffer> _pUploadBuffer;
	mutable std::bitset<kFrameResourceCount> _bufferDirty;
};


/*****************************************************************************************************/


template<typename T>
class FRConstantBuffer : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, const T &object);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	size_t getBufferSize() const override;
	size_t getElementStride() const override;
	BYTE *getMappedPtr() override;
	const BYTE *getMappedPtr() const override;
	void updateBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	ConstantBufferView getCBV() const override;
	T *map();
	const T *cmap() const;
	CBufferVisitor<T> visit();
	CBufferVisitor<const T> visit() const;
private:
	T _object;
	DescriptorAllocation _descriptor;
	mutable std::unique_ptr<UploadBuffer> _pUploadBuffer;
	mutable std::bitset<kFrameResourceCount> _bufferDirty;
};

/*****************************************************************************************************/

template <typename T>
FRConstantBuffer<T>::FRConstantBuffer(std::weak_ptr<Device> pDevice, const T &object) : _object(object) {
	auto pSharedDevice = pDevice.lock();
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		static_cast<size_t>(kFrameResourceCount),
		sizeof(T),
		true
	);

	_descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		_bufferDirty.set(i, true);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
		cbv.BufferLocation = _pUploadBuffer->getGPUAddressByIndex(i);
		cbv.SizeInBytes = static_cast<UINT>(UploadBuffer::calcConstantBufferByteSize(sizeof(T)));
		pSharedDevice->getD3DDevice()->CreateConstantBufferView(
			&cbv,
			_descriptor.getCPUHandle(i)
		);
	}
}

template <typename T>
WRL::ComPtr<ID3D12Resource> FRConstantBuffer<T>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

template <typename T>
size_t FRConstantBuffer<T>::getBufferSize() const {
	return _pUploadBuffer->getElementByteSize();
}

template <typename T>
size_t FRConstantBuffer<T>::getElementStride() const {
	return sizeof(T);
}

template <typename T>
BYTE *FRConstantBuffer<T>::getMappedPtr() {
	return _pUploadBuffer->getMappedDataByIndex(FrameIndexProxy::getConstantFrameIndexRef());
}

template <typename T>
const BYTE *FRConstantBuffer<T>::getMappedPtr() const {
	return _pUploadBuffer->getMappedDataByIndex(FrameIndexProxy::getConstantFrameIndexRef());
}

template <typename T>
void FRConstantBuffer<T>::updateBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	assert((sizeInByte + offset) <= sizeof(T));
	std::memcpy(reinterpret_cast<char *>(& _object) + offset, pData, sizeInByte);
	_bufferDirty.set(FrameIndexProxy::getConstantFrameIndexRef());
}

template <typename T>
ConstantBufferView FRConstantBuffer<T>::getCBV() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	if (_bufferDirty.test(frameIndex)) {
		_pUploadBuffer->copyData(frameIndex, &_object, sizeof(T), 0);
		_bufferDirty.set(frameIndex, true);
	}
	return ConstantBufferView(_descriptor, frameIndex);
}

template <typename T>
T *FRConstantBuffer<T>::map() {
	return &_object;
}

template <typename T>
const T *FRConstantBuffer<T>::cmap() const {
	return &_object;
}

template <typename T>
CBufferVisitor<T> FRConstantBuffer<T>::visit() {
	return CBufferVisitor<T>(map());
}

template <typename T>
CBufferVisitor<const T> FRConstantBuffer<T>::visit() const {
	return CBufferVisitor<const T>(cmap());
}

}

