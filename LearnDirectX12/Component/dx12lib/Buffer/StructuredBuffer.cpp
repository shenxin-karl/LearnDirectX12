#include "StructuredBuffer.h"
#include "Device.h"
#include "UploadBuffer.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> StructuredBuffer::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

size_t StructuredBuffer::getBufferSize() const {
	return _pUploadBuffer->getBufferSize();
}

size_t StructuredBuffer::getElementCount() const {
	return getBufferSize() / _elementStride;
}

size_t StructuredBuffer::getElementStride() const {
	return _elementStride;
}

BYTE *StructuredBuffer::getMappedPtr() {
	return _pUploadBuffer->getMappedDataByIndex(0);
}

const BYTE *StructuredBuffer::getMappedPtr() const {
	return _pUploadBuffer->getMappedDataByIndex(0);
}

void StructuredBuffer::updateBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	assert((sizeInByte + offset) <= getBufferSize());
	_pUploadBuffer->copyData(0, pData, sizeInByte, offset);
}

ShaderResourceView StructuredBuffer::getSRV() const {
	return ShaderResourceView(_descriptor);
}

StructuredBuffer::~StructuredBuffer() {
}

StructuredBuffer::StructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride)
: _elementStride(stride)
{
	auto pSharedDevice = pDevice.lock();
	size_t sizeInByte = numElements * stride;
	_descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_pUploadBuffer = std::make_unique<UploadBuffer>(
		pSharedDevice->getD3DDevice(),
		1,
		sizeInByte,
		false
	);

	auto *pDest = _pUploadBuffer->getMappedDataByIndex();
	if (pData != nullptr)
		std::memcpy(pDest, pData, sizeInByte);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = _pUploadBuffer->getD3DResource()->GetDesc().Format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.StructureByteStride = static_cast<UINT>(stride);

	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = static_cast<UINT>(numElements);

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pUploadBuffer->getD3DResource().Get(),
		&desc,
		_descriptor.getCPUHandle()
	);
}

}