#include "dx12libCommon.h"

namespace dx12lib {

class UploadBuffer {
public:
	UploadBuffer(ID3D12Device *pDevice, UINT elementCount, UINT elementByteSize, bool isConstantBuffer);
	UploadBuffer(const UploadBuffer &) = delete;
	UploadBuffer(UploadBuffer &&other) noexcept;
	UploadBuffer &operator=(UploadBuffer &&other) noexcept;
	void copyData(UINT elementIndex, const void *pData);
	D3D12_GPU_VIRTUAL_ADDRESS getGPUAddressByIndex(UINT elementIndex = 0) const;
	uint32 getElementByteSize() const noexcept;
	uint32 getElementCount() const noexcept;
	~UploadBuffer();
	static UINT calcConstantBufferByteSize(std::size_t size) noexcept;
	friend void swap(UploadBuffer &lhs, UploadBuffer &rhs) noexcept;
private:
	UploadBuffer();
	WRL::ComPtr<ID3D12Resource>  _pUploadBuffer;
	BYTE  *_pMappedData;
	UINT   _elementByteSize;
	UINT   _elementCount;
	bool   _isConstantBuffer;
};

}