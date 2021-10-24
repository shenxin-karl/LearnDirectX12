#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include "d3dulti.h"
#include "D3DX12.h"

namespace WRL = Microsoft::WRL;

template<typename T>
class UploadBuffer {
	UploadBuffer(ID3D12Device *device, UINT elementCout, bool isConstanBuffer);
	UploadBuffer(const UploadBuffer &) = delete;
	UploadBuffer &operator(const UploadBuffer &) = delete;
	ID3D12Resource *resource() const;
	void copyData(int elementIndex, const T &data);
	~UploadBuffer();
private:
	WRL::ComPtr<ID3D12Resource>		uploadBuffer_;
	BYTE						   *mappedData_;
	UINT							elementByteSize_;
	bool							isConstanBuffer_;
};

template<typename T>
UploadBuffer<T>::UploadBuffer(ID3D12Device *device, UINT elementCout, bool isConstanBuffer)
: isConstanBuffer_(isConstanBuffer) {
	elementByteSize_ = sizeof(T);
	if (isConstanBuffer_)
		elementByteSize_ = calcConstantBufferByteSize(sizeof(T));

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto buffer = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize_ * elementCout);
	ThrowIfFailed(device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&buffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer_)
	));

	ThrowIfFailed(uploadBuffer_->Map(0, nullptr, reinterpret_cast<void **>(&mappedData_)));
}


template<typename T>
ID3D12Resource *UploadBuffer<T>::resource() const {
	ID3D12Resource *res = uploadBuffer_.Get();
	assert(res != nullptr);
	return res;
}


template<typename T>
void UploadBuffer<T>::copyData(int elementIndex, const T &data) {
	assert(mappedData_ != nullptr);
	memcpy(&mappedData_[elementIndex], &data, sizeof(T));
}

template<typename T>
UploadBuffer<T>::~UploadBuffer() {
	if (uploadBuffer_ != nullptr)
		uploadBuffer_->Unmap(0, nullptr);
}
