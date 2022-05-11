#include <dx12lib/Buffer/ConsumeStructuredBuffer.h>

#include "DefaultBuffer.h"
#include "dx12lib/Context/CommandList.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> ConsumeStructuredBuffer::getD3DResource() const {
	return _pDefaultBuffer->getD3DResource();
}

size_t ConsumeStructuredBuffer::getBufferSize() const {
	return _pDefaultBuffer->getBufferSize();
}

size_t ConsumeStructuredBuffer::getElementCount() const {
	return _pDefaultBuffer->getBufferSize() / _elementStride;
}

size_t ConsumeStructuredBuffer::getElementStride() const {
	return _elementStride;
}

UnorderedAccessView ConsumeStructuredBuffer::getUAV() const {
	return _uav;
}

ConsumeStructuredBuffer::ConsumeStructuredBuffer(std::weak_ptr<Device> pDevice,
	std::shared_ptr<CommandList> pCmdList,
	const void *pData,
	size_t numElements,
	size_t stride)
: _elementStride(stride)
{
	auto pSharedDevice = pDevice.lock();
	size_t sizeInByte = numElements * stride;
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(pSharedDevice->getD3DDevice(),
		pCmdList->getD3DCommandList(),
		pData,
		sizeInByte,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

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
		_pDefaultBuffer->getD3DResource().Get(),
		nullptr,
		&uavDesc,
		descriptor.getCPUHandle()
	);

	_uav = UnorderedAccessView(descriptor, this);
}

}
