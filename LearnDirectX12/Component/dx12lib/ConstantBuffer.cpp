#include "ConstantBuffer.h"
#include "UploadBuffer.h"
#include "Device.h"
#include "CommandList.h"
#include "DescriptorAllocation.h"

namespace dx12lib {


ConstantBuffer::ConstantBuffer(const ConstantBufferDesc &desc)
: _frameIndex(desc.frameIndex)
{
	_bufferSize = desc.sizeInByte;
	_pGPUBuffer = std::make_unique<UploadBuffer>(
		desc.pDevice.lock()->getD3DDevice(),
		desc.frameCount,
		desc.sizeInByte,
		true
	);

	auto bufferSizes = UploadBuffer::calcConstantBufferByteSize(desc.sizeInByte);
	auto pShadedDevice = desc.pDevice.lock();
	auto *pD3DDevice = pShadedDevice->getD3DDevice();
	for (uint32 i = 0; i < desc.frameCount; ++i) {
		// padding buffer data
		auto *pMappedPtr = _pGPUBuffer->getMappedDataByIndex(i);
		if (desc.pData == nullptr) {
			std::memset(pMappedPtr, 0, bufferSizes);
		} else {
			std::memcpy(pMappedPtr, desc.pData, desc.sizeInByte);
			pMappedPtr += desc.sizeInByte;
			if (auto leftSize = (bufferSizes - desc.sizeInByte))
				std::memset(pMappedPtr, 0, leftSize);
		}

		// create constant buffer view
		auto CBV = pShadedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferDesc = {
			_pGPUBuffer->getGPUAddressByIndex(i),
			bufferSizes,
		};
		pD3DDevice->CreateConstantBufferView(
			&constantBufferDesc,
			CBV.getCPUHandle()
		);
		_CBV.push_back(std::move(CBV));
	}
}

void ConstantBuffer::updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) {
	assert(offset + sizeInByte < _bufferSize);
	BYTE *pDest = getMappedPtr();
	pDest += offset;
	std::memcpy(pDest, pData, sizeInByte);
}

BYTE *ConstantBuffer::getMappedPtr() {
	return _pGPUBuffer->getMappedDataByIndex(_frameIndex);
}

BYTE *ConstantBuffer::getMappedPtr() const {
	return _pGPUBuffer->getMappedDataByIndex(_frameIndex);
}

uint32 ConstantBuffer::getConstantBufferSize() const noexcept {
	return _bufferSize;
}

uint32 ConstantBuffer::getConstantAlignedBufferSize() const noexcept {
	return _pGPUBuffer != nullptr ? _pGPUBuffer->getElementByteSize() : 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::getConstantBufferView() const {
	return _CBV[_frameIndex].getCPUHandle();
}

}