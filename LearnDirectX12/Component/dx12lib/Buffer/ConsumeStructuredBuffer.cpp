#include <dx12lib/Buffer/ConsumeStructuredBuffer.h>

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> ConsumeStructuredBuffer::getD3DResource() const {
	return _pUploadBuffer->getD3DResource();
}

size_t ConsumeStructuredBuffer::getBufferSize() const {
	return _pUploadBuffer->getBufferSize();
}

size_t ConsumeStructuredBuffer::getElementCount() const {
	return _pUploadBuffer->getBufferSize() / _elementStride;
}

size_t ConsumeStructuredBuffer::getElementStride() const {
	return _elementStride;
}

UnorderedAccessView ConsumeStructuredBuffer::getUAV() const {
	return _uav;
}

void ConsumeStructuredBuffer::updateBuffer(const void *pData, size_t sizeInByte, size_t offset) {
	_pUploadBuffer->copyData(0, pData, sizeInByte, offset);
}

ConsumeStructuredBuffer::ConsumeStructuredBuffer(std::weak_ptr<Device> pDevice, 
	const void *pData,
	size_t numElements,
	size_t stride)
: _elementStride(stride)
{
	auto pSharedDevice = pDevice.lock();
	size_t sizeInByte = numElements * stride;
	_pUploadBuffer = std::make_unique<UploadBuffer>(pSharedDevice->getD3DDevice(),
		1,
		sizeInByte,
		false,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	if (pData != nullptr)
		_pUploadBuffer->copyData(0, pData);

	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = static_cast<UINT>(numElements);
	uavDesc.Buffer.StructureByteStride = static_cast<UINT>(stride);
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	pSharedDevice->getD3DDevice()->CreateUnorderedAccessView(
		_pUploadBuffer->getD3DResource().Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);

	_uav = UnorderedAccessView(descriptor, this);
}

ConsumeStructuredBuffer::ConsumeStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, size_t stride)
: ConsumeStructuredBuffer(pDevice, nullptr, numElements, stride) {
}

}
