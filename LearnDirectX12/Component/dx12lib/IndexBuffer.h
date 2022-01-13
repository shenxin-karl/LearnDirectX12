#pragma once
#include "dx12libCommon.h"
#include <memory>
namespace dx12lib {

class DefaultBuffer;
class IndexBuffer {
public:
	IndexBuffer();
	IndexBuffer(ID3D12Device *pDevice, 
		ID3D12GraphicsCommandList *pCmdList, 
		const void *pData,
		uint32 sizeInByte, 
		DXGI_FORMAT format
	);
	IndexBuffer(const IndexBuffer &) = delete;
	IndexBuffer(IndexBuffer &&other) noexcept;
	IndexBuffer &operator=(IndexBuffer &&other) noexcept;
	~IndexBuffer() = default;
	friend void swap(IndexBuffer &lhs, IndexBuffer &rhs) noexcept;
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const noexcept;
	DXGI_FORMAT getIndexFormat() const noexcept;
	uint32 getIndexBufferSize() const noexcept;
	bool isEmpty() const noexcept;
private:
	std::unique_ptr<DefaultBuffer> _pGPUBuffer;
	WRL::ComPtr<ID3DBlob> _pCPUBuffer;
	DXGI_FORMAT _indexFormat;
	uint32 _indexBufferByteSize;
};

}