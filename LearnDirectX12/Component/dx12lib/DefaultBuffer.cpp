#include "DefaultBuffer.h"
#include "ResourceStateTracker.h"

namespace dx12lib {

DefaultBuffer::DefaultBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, const void *pData, uint32 sizeInByte)
{
	assert(pDevice != nullptr && "createDefaultBuffer pDevice is nullptr");
	assert(pCmdList != nullptr && "createDefaultBuffer pCmdList is nullptr");

	ThrowIfFailed(pDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(sizeInByte)),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&_pDefaultBuffer)
	));

	// create upload heap
	ThrowIfFailed(pDevice->CreateCommittedResource(
		RVPtr(CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
		D3D12_HEAP_FLAG_NONE,
		RVPtr(CD3DX12_RESOURCE_DESC::Buffer(sizeInByte)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_pUploader)
	));

	// describes the data we want to copy to the default buffer
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pData;
	subResourceData.RowPitch = sizeInByte;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// copy the data to upload heap using the UpdateResources function
	pCmdList->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		_pDefaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST
	)));

	UpdateSubresources(pCmdList, _pDefaultBuffer.Get(), _pUploader.Get(), 0, 0, 1, &subResourceData);
	pCmdList->ResourceBarrier(1, RVPtr(CD3DX12_RESOURCE_BARRIER::Transition(
		_pDefaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ
	)));

	ResourceStateTracker::addGlobalResourceState(_pDefaultBuffer.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

DefaultBuffer::DefaultBuffer(DefaultBuffer &&other) noexcept : DefaultBuffer() {
	swap(*this, other);
}

D3D12_GPU_VIRTUAL_ADDRESS DefaultBuffer::getAddress() const {
	return _pDefaultBuffer->GetGPUVirtualAddress();
}

WRL::ComPtr<ID3D12Resource> DefaultBuffer::getD3DResource() const {
	return _pDefaultBuffer;
}

DefaultBuffer &DefaultBuffer::operator=(DefaultBuffer &&other) noexcept {
	DefaultBuffer tmp;
	swap(*this, tmp);
	swap(*this, other);
	return *this;
}

void swap(DefaultBuffer &lhs, DefaultBuffer &rhs) noexcept {
	using std::swap;
	swap(lhs._pDefaultBuffer, rhs._pDefaultBuffer);
	swap(lhs._pUploader, rhs._pUploader);
}


}