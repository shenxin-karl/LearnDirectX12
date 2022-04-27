#include "StructuredBuffer.h"
#include "Device.h"
#include "UploadBuffer.h"

namespace dx12lib {

ShaderResourceView StructuredBuffer::getShaderResourceView() const {
	return ShaderResourceView(_descriptor);
}

WRL::ComPtr<ID3D12Resource> StructuredBuffer::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

void StructuredBuffer::updateStructuredBuffer(const void* pData, size_t sizeInByte, size_t offset) {

}

size_t StructuredBuffer::getStructuredBufferSize() const {
	return _pUploadBuffer->getElementByteSize();
}

size_t StructuredBuffer::getElementCount() const {
	return getStructuredBufferSize() / getStride();
}

size_t StructuredBuffer::getStride() const {
	return _pUploadBuffer->getElementByteSize();
}

StructuredBuffer::~StructuredBuffer() {
}

StructuredBuffer::StructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride) {
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