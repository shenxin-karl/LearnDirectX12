#pragma once
#include "Device.h"
#include "IResource.h"
#include "UploadBuffer.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

template<>
class FRConstantBuffer<RawData> : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void *pData);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
	void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getConstantBufferSize() const noexcept override;
	void *map();
	const void *cmap() const;
private:
	DescriptorAllocation _CBV;
	std::unique_ptr<BYTE[]> _pObject;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
	mutable std::bitset<kFrameResourceCount> _bufferDirty;
};


/*****************************************************************************************************/


template<typename T>
class FRConstantBuffer : public IConstantBuffer {
protected:
	FRConstantBuffer(std::weak_ptr<Device> pDevice, const T &object);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	bool isMapped() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;
	void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset) override;
	size_t getConstantBufferSize() const noexcept override;
	T *map();
	const T *cmap() const;
private:
	T _object;
	DescriptorAllocation _CBV;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
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

	_CBV = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		_bufferDirty.set(i, true);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
		cbv.BufferLocation = _pUploadBuffer->getGPUAddressByIndex(i);
		cbv.SizeInBytes = static_cast<UINT>(UploadBuffer::calcConstantBufferByteSize(sizeof(T)));
		pSharedDevice->getD3DDevice()->CreateConstantBufferView(
			&cbv,
			_CBV.getCPUHandle(i)
		);
	}
	_resourceType = ResourceType::ConstantBuffer;
}

template <typename T>
WRL::ComPtr<ID3D12Resource> FRConstantBuffer<T>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

template <typename T>
bool FRConstantBuffer<T>::isMapped() const {
	return true;
}

template <typename T>
D3D12_CPU_DESCRIPTOR_HANDLE FRConstantBuffer<T>::getConstantBufferView() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	if (_bufferDirty.test(frameIndex)) {
		_pUploadBuffer->copyData(frameIndex, &_object, getConstantBufferSize(), 0);
		_bufferDirty.set(frameIndex, false);
	}
	return _CBV.getCPUHandle(frameIndex);
}

template <typename T>
void FRConstantBuffer<T>::updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	_pUploadBuffer->copyData(frameIndex, pData, sizeInByte, offset);
}

template <typename T>
size_t FRConstantBuffer<T>::getConstantBufferSize() const noexcept {
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

