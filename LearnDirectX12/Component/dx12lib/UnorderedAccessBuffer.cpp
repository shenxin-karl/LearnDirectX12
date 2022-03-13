#include "UnorderedAccessBuffer.h"
#include "Device.h"
#include "D3Dx12.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> UnorderedAccessBuffer::getD3DResource() const {
	return _pResource;
}

std::size_t UnorderedAccessBuffer::getBufferSize() const {
	return _bufferSize;
}

UnorderedAccessBuffer::~UnorderedAccessBuffer() {
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

UnorderedAccessBuffer::UnorderedAccessBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte) {
	_bufferSize = sizeInByte;
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(
			sizeInByte,
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		)),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&_pResource)
	));

	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

}