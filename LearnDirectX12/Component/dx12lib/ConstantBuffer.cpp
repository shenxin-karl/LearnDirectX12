#include "ConstantBuffer.h"
#include "UploadBuffer.h"
#include "Device.h"
#include "CommandList.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

ConstantBuffer::ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t sizeInByte) {
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pDevice.lock()->getD3DDevice(),
		1,
		sizeInByte,
		true
	);

	if (pData != nullptr) {
		auto *pMapped = _pUploadBuffer->getMappedDataByIndex(0);
		std::memcpy(pMapped, pData, sizeInByte);
	}

	auto address = _pUploadBuffer->getGPUAddressByIndex(0);
	auto pSharedDevice = pDevice.lock();
	_CBV = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
	cbv.BufferLocation = address;
	cbv.SizeInBytes = static_cast<UINT>(UploadBuffer::calcConstantBufferByteSize(sizeInByte));
	pSharedDevice->getD3DDevice()->CreateConstantBufferView(&cbv, _CBV.getCPUHandle());
}

WRL::ComPtr<ID3D12Resource> ConstantBuffer::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

void ConstantBuffer::updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	assert((offset + sizeInByte) < getConstantBufferSize());
	BYTE *pDest = getMappedPtr();
	pDest += offset;
	std::memcpy(pDest, pData, sizeInByte);
}

BYTE *ConstantBuffer::getMappedPtr() {
	return _pUploadBuffer->getMappedDataByIndex(0);
}

const BYTE *ConstantBuffer::getMappedPtr() const {
	return _pUploadBuffer->getMappedDataByIndex(0);
}

size_t ConstantBuffer::getConstantBufferSize() const noexcept {
	return _pUploadBuffer->getElementByteSize();
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::getConstantBufferView() const {
	return _CBV.getCPUHandle();
}

}