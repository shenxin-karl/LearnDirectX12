#include "FRConstantBuffer.hpp"

namespace dx12lib {

FRConstantBuffer<RawData>::FRConstantBuffer(std::weak_ptr<Device> pDevice, size_t sizeInByte, const void *pData) {
	_pObject = std::unique_ptr<BYTE[]>(new BYTE[sizeInByte]);
	if (pData != nullptr)
		std::memcpy(_pObject.get(), pData, sizeInByte);
	else
		std::memset(_pObject.get(), 0, sizeInByte);

	auto pSharedDevice = pDevice.lock();
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		static_cast<size_t>(kFrameResourceCount),
		sizeInByte,
		true
	);

	_descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kFrameResourceCount);
	for (size_t i = 0; i < kFrameResourceCount; ++i) {
		_bufferDirty.set(i, true);
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
		cbv.BufferLocation = _pUploadBuffer->getGPUAddressByIndex(i);
		cbv.SizeInBytes = static_cast<UINT>(UploadBuffer::calcConstantBufferByteSize(sizeInByte));
		pSharedDevice->getD3DDevice()->CreateConstantBufferView(
			&cbv,
			_descriptor.getCPUHandle(i)
		);
	}
}

WRL::ComPtr<ID3D12Resource> FRConstantBuffer<RawData>::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

bool FRConstantBuffer<RawData>::isMapped() const {
	return true;
}

ConstantBufferView FRConstantBuffer<RawData>::getConstantBufferView() const {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	if (_bufferDirty.test(frameIndex)) {
		_pUploadBuffer->copyData(frameIndex, _pObject.get(), getConstantBufferSize(), 0);
		_bufferDirty.set(frameIndex, false);
	}
	return ConstantBufferView(_descriptor, frameIndex);
}

void FRConstantBuffer<RawData>::updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
	_pUploadBuffer->copyData(frameIndex, pData, sizeInByte, offset);
}

size_t FRConstantBuffer<RawData>::getConstantBufferSize() const noexcept {
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

}
