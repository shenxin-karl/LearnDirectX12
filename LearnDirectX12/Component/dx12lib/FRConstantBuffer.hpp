#pragma once
#include "Device.h"
#include "IResource.h"
#include "UploadBuffer.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

// frame resource constant buffer template
template<typename T = RawData>
class FRConstantBuffer;

template<>
class FRConstantBuffer<RawData> : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, std::atomic_int32_t &frmeIndex, uint32 sizeInByte);
public:
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) override;
	uint32 getConstantBufferSize() const noexcept override;
	void *map();
	const void *cmap() const;
private:
	DescriptorAllocation _CBV;
	std::unique_ptr<BYTE[]> _pObject;
	const std::atomic_int32_t &_frameIndex;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
	mutable std::bitset<kFrameResourceCount> _bufferDirty;
};


/*****************************************************************************************************/


template<typename T>
class FRConstantBuffer : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, std::atomic_int32_t &frmeIndex, uint32 sizeInByte = sizeof(T));
public:
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
	void updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) override;
	uint32 getConstantBufferSize() const noexcept override;
	T *map();
	const T *cmap() const;
private:
	T _object;
	DescriptorAllocation _CBV;
	const std::atomic_int32_t &_frameIndex;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
	mutable std::bitset<kFrameResourceCount> _bufferDirty;
};


/*****************************************************************************************************/


FRConstantBuffer<RawData>::FRConstantBuffer(std::weak_ptr<Device> pDevice,
	std::atomic_int32_t &frmeIndex,
	uint32 sizeInByte)
	: _frameIndex(frmeIndex) {
	_pObject = std::unique_ptr<BYTE[]>(new BYTE[sizeInByte]);
	std::memset(_pObject.get(), 0, sizeInByte);

	auto pSharedDevice = pDevice.lock();
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		kFrameResourceCount,
		sizeInByte,
		true
		);

	_CBV = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (std::size_t i = 0; i < kFrameResourceCount; ++i) {
		_bufferDirty.set(i, true);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
		cbv.BufferLocation = _pUploadBuffer->getGPUAddressByIndex(i);
		cbv.SizeInBytes = UploadBuffer::calcConstantBufferByteSize(sizeInByte);
		pSharedDevice->getD3DDevice()->CreateConstantBufferView(
			&cbv,
			_CBV.getCPUHandle(i)
		);
	}
}

bool FRConstantBuffer<RawData>::isMapped() const {
	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE FRConstantBuffer<RawData>::getConstantBufferView() const {
	if (_bufferDirty.test(_frameIndex)) {
		_pUploadBuffer->copyData(_frameIndex, _pObject.get(), getConstantBufferSize(), 0);
		_bufferDirty.set(_frameIndex, false);
	}
	return _CBV.getCPUHandle(_frameIndex);
}

void FRConstantBuffer<RawData>::updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) {
	_pUploadBuffer->copyData(_frameIndex, pData, sizeInByte, offset);
}

uint32 FRConstantBuffer<RawData>::getConstantBufferSize() const noexcept {
	return _pUploadBuffer->getElementByteSize();
}

void *FRConstantBuffer<RawData>::map() {
	for (std::size_t i = 0; i < kFrameResourceCount; ++i)
		_bufferDirty.set(i, true);
	return _pObject.get();
}

const void *FRConstantBuffer<RawData>::cmap() const {
	return _pObject.get();
}

/*****************************************************************************************************/

template <typename T>
bool FRConstantBuffer<T>::isMapped() const {
	return true;
}

template <typename T>
D3D12_CPU_DESCRIPTOR_HANDLE FRConstantBuffer<T>::getConstantBufferView() const {
	if (_bufferDirty.test(_frameIndex)) {
		_pUploadBuffer->copyData(_frameIndex, &_object, getConstantBufferSize(), 0);
		_bufferDirty.set(_frameIndex, false);
	}
	return _CBV.getCPUHandle(_frameIndex);
}

template <typename T>
void FRConstantBuffer<T>::updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) {
	_pUploadBuffer->copyData(_frameIndex, pData, sizeInByte, offset);
}

template <typename T>
uint32 FRConstantBuffer<T>::getConstantBufferSize() const noexcept {
	return _pUploadBuffer->getElementByteSize();
}

template <typename T>
T *FRConstantBuffer<T>::map() {
	for (std::size_t i = 0; i < kFrameResourceCount; ++i)
		_bufferDirty.set(i, true);
	return &_object;
}

template <typename T>
const T *FRConstantBuffer<T>::cmap() const {
	return &_object;
}

}

