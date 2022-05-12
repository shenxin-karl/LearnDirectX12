#include <dx12lib/Buffer/AppendStructuredBuffer.h>

#include "DefaultBuffer.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> AppendStructuredBuffer::getD3DResource() const {
	return _pDefaultBuffer->getD3DResource();
}

size_t AppendStructuredBuffer::getBufferSize() const {
	return _pDefaultBuffer->getBufferSize();
}

size_t AppendStructuredBuffer::getElementCount() const {
	return getBufferSize() / _elementStride;
}

size_t AppendStructuredBuffer::getElementStride() const {
	return _elementStride;
}

UnorderedAccessView AppendStructuredBuffer::getUAV() const {
	return _uav;
}

ShaderResourceView AppendStructuredBuffer::getSRV() const {
	return _srv;
}

AppendStructuredBuffer::AppendStructuredBuffer(std::weak_ptr<Device> pDevice, size_t numElements, size_t stride)
: _elementStride(stride)
{
	auto pSharedDevice = pDevice.lock();
	size_t sizeInByte = numElements * stride;
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(
		pSharedDevice->getD3DDevice(),
		nullptr,
		nullptr,
		sizeInByte,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS
	);

	auto descriptor = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

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
		descriptor.getCPUHandle(0)
	);
	_uav = UnorderedAccessView(descriptor, this, 0);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = static_cast<UINT>(numElements);
	srvDesc.Buffer.StructureByteStride = static_cast<UINT>(stride);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pDefaultBuffer->getD3DResource().Get(),
		&srvDesc,
		descriptor.getCPUHandle(1)
	);
	_srv = ShaderResourceView(descriptor, this, 1);
}

}
