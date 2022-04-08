#pragma once
#include "dx12libStd.h"
#include "IResource.h"

namespace dx12lib {

class UploadBuffer : public IResource {
public:
	UploadBuffer(ID3D12Device *pDevice, size_t elementCount, size_t elementByteSize, bool isConstantBuffer);
	UploadBuffer(UploadBuffer &&other) noexcept;
	UploadBuffer &operator=(UploadBuffer &&other) noexcept;
	void copyData(size_t elementIndex, const void *pData);
	void copyData(size_t elementIndex, const void *pData, size_t sizeInByte, size_t offset);
	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddressByIndex(size_t elementIndex = 0) const;
	BYTE *getMappedDataByIndex(size_t elementIndex = 0);
	const BYTE *getMappedDataByIndex(size_t elementIndex = 0) const;
	size_t getElementByteSize() const noexcept;
	size_t getElementCount() const noexcept;
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	~UploadBuffer() override;
	static size_t calcConstantBufferByteSize(std::size_t size) noexcept;
	friend void swap(UploadBuffer &lhs, UploadBuffer &rhs) noexcept;
private:
	UploadBuffer();
	WRL::ComPtr<ID3D12Resource>  _pUploadBuffer;
	BYTE  *_pMappedData;
	bool   _isConstantBuffer;
	size_t _elementByteSize;
	size_t _elementCount;
};

}