#include "ReadBackBuffer.h"
#include "ResourceStateTracker.h"
#include "Device.h"

namespace dx12lib {

WRL::ComPtr<ID3D12Resource> ReadBackBuffer::getD3DResource() const {
	return _pResource;
}

ReadBackBuffer::~ReadBackBuffer() {
	if (_pMapped != nullptr) {
		_pResource->Unmap(0, nullptr);
		_pMapped = nullptr;
	}
	ResourceStateTracker::removeGlobalResourceState(_pResource.Get());
}

bool ReadBackBuffer::isCompleted() const {
	return _isCompleted;
}

bool ReadBackBuffer::isMapped() const {
	return _pMapped != nullptr;
}

const void *ReadBackBuffer::getMapped() const {
	if (isCompleted() && _pMapped == nullptr) 
		ThrowIfFailed(_pResource->Map(0, nullptr, &_pMapped));

	return _pMapped;
}

ReadBackBuffer::ReadBackBuffer(std::weak_ptr<Device> pDevice, std::size_t sizeInByte) : _bufferSize(sizeInByte) {
	ThrowIfFailed(pDevice.lock()->getD3DDevice()->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(sizeInByte)),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&_pResource)
	));
	_resourceType = ResourceType::ReadBackBuffer;
	ResourceStateTracker::addGlobalResourceState(_pResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
}

void ReadBackBuffer::setCompleted(bool flag) {
	_isCompleted = flag;
}

}