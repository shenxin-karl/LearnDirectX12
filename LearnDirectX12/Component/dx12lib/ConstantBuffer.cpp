#include "ConstantBuffer.h"
#include "UploadBuffer.h"
#include "Device.h"

namespace dx12lib {

ConstantBuffer::ConstantBuffer() : _bufferSize(0) {
}

ConstantBuffer::ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, uint32 sizeInByte) {
	_bufferSize = sizeInByte;
	_pGPUBuffer = std::make_unique<UploadBuffer>(pDevice.lock()->getD3DDevice(), 1, sizeInByte, true);
	_CBV = pDevice.lock()->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
}

ConstantBuffer::ConstantBuffer(ConstantBuffer &&other) noexcept : ConstantBuffer() {
	swap(*this, other);
}

ConstantBuffer &ConstantBuffer::operator=(ConstantBuffer &&other) noexcept {
	ConstantBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void ConstantBuffer::updateConstantBuffer(const void *pData, uint32 sizeInByte, uint32 offset) {
	assert(offset + sizeInByte < _bufferSize);
	//_pGPUBuffer->copyData()
	// TODO:  Some data needs to be updated
}

uint32 ConstantBuffer::getConstantBufferSize() const noexcept {
	return _bufferSize;
}

uint32 ConstantBuffer::getConstantAlignedBufferSize() const noexcept {
	return _pGPUBuffer != nullptr ? _pGPUBuffer->getElementByteSize() : 0;
}


D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::getConstantBufferView() const {
	return _CBV.getCPUHandle();
}

bool ConstantBuffer::isEmpty() const noexcept {
	return _pGPUBuffer == nullptr;
}

void swap(ConstantBuffer &lhs, ConstantBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pGPUBuffer, rhs._pGPUBuffer);
	swap(lhs._bufferSize, rhs._bufferSize);
}


}