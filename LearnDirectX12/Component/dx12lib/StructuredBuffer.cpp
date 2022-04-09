#include "StructuredBuffer.h"
#include "Device.h"
#include "UploadBuffer.h"

namespace dx12lib {

bool StructuredBuffer::isMapped() const {
	return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE StructuredBuffer::getShaderResourceView() const {
	return _structedBufferView.getCPUHandle();
}

WRL::ComPtr<ID3D12Resource> StructuredBuffer::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

void StructuredBuffer::updateStructuredBuffer(const void* pData, size_t sizeInByte, size_t offset) {

}

size_t StructuredBuffer::getStructuredBufferSize() const {
	return _pUploadBuffer->getElementByteSize();
}

StructuredBuffer::~StructuredBuffer() {
}

StructuredBuffer::StructuredBuffer(std::weak_ptr<Device> pDevice, const void *pData, size_t numElements, size_t stride)
{
	auto pSharedDevice = pDevice.lock();
	size_t sizeInByte = numElements * stride;
	_structedBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
	desc.Buffer.StructureByteStride = stride;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = numElements;

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pUploadBuffer->getD3DResource().Get(),
		nullptr,
		_structedBufferView.getCPUHandle()
	);
	_resourceType = ResourceType::StructuredBuffer;
}

}