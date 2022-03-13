#include "StructedBuffer.h"
#include "DefaultBuffer.h"
#include "Device.h"
#include "CommandList.h"

namespace dx12lib {

D3D12_CPU_DESCRIPTOR_HANDLE StructedBuffer::getStructedBufferView() const {
	return _structedBufferView.getCPUHandle();
}

WRL::ComPtr<ID3D12Resource> StructedBuffer::getD3DResource() const {
	return _pDefaultBuffer->getD3DResource();
}

std::size_t StructedBuffer::getBufferSize() const {
	return _bufferSize;
}

StructedBuffer::StructedBuffer(std::weak_ptr<Device> pDevice, 
	std::shared_ptr<CommandList> pCmdList, 
	const void *pData, 
	std::size_t sizeInByte) 
{
	auto pSharedDevice = pDevice.lock();
	_structedBufferView = pSharedDevice->allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_bufferSize = sizeInByte;
	_pDefaultBuffer = std::make_unique<DefaultBuffer>(
		pSharedDevice->getD3DDevice(),
		pCmdList->getD3DCommandList(),
		pData,
		sizeInByte
	);

	// create view
	pSharedDevice->getD3DDevice()->CreateShaderResourceView(
		_pDefaultBuffer->getD3DResource().Get(),
		nullptr,
		_structedBufferView.getCPUHandle()
	);
}

}