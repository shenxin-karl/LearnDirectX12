#include "ConstantBuffer.h"
#include "UploadBuffer.h"
#include "Device.h"
#include "CommandList.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

ConstantBuffer::ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, uint32 sizeInByte)
: _bufferSize(sizeInByte)
{
	_pGPUBuffer = std::make_unique<UploadBuffer>(
		pDevice.lock()->getD3DDevice(),
		1,
		_bufferSize,
		true
	);

	if (pData != nullptr) {
		auto *pMapped = _pGPUBuffer->getMappedDataByIndex(0);
		std::memcpy(pMapped, pData, _bufferSize);
	}

	auto address = _pGPUBuffer->getGPUAddressByIndex(0);
	auto pSharedDevice = pDevice.lock();
	_CBV = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
	cbv.BufferLocation = address;
	cbv.SizeInBytes = UploadBuffer::calcConstantBufferByteSize(_bufferSize);
	pSharedDevice->getD3DDevice()->CreateConstantBufferView(&cbv, _CBV.getCPUHandle());
}

WRL::ComPtr<ID3D12Resource> ConstantBuffer::getD3DResource() const {
	return _pGPUBuffer->getD3DResource();
}

void ConstantBuffer::updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) {
	assert((offset + sizeInByte) <= _bufferSize);
	BYTE *pDest = getMappedPtr();
	pDest += offset;
	std::memcpy(pDest, pData, sizeInByte);
}

BYTE *ConstantBuffer::getMappedPtr() {
	return _pGPUBuffer->getMappedDataByIndex(0);
}

const BYTE *ConstantBuffer::getMappedPtr() const {
	return _pGPUBuffer->getMappedDataByIndex(0);
}

uint32 ConstantBuffer::getConstantBufferSize() const noexcept {
	return _bufferSize;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::getConstantBufferView() const {
	return _CBV.getCPUHandle();
}

}