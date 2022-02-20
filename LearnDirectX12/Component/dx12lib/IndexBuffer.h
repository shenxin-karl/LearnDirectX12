#pragma once
#include "dx12libStd.h"
#include "IResource.h"
#include <memory>

namespace dx12lib {

class IndexBuffer : public IResource {
protected:
	IndexBuffer();
	IndexBuffer(std::weak_ptr<Device> pDevice, 
		std::shared_ptr<CommandList> pCmdList, 
		const void *pData,
		uint32 sizeInByte, 
		DXGI_FORMAT format
	);
public:
	IndexBuffer(const IndexBuffer &) = delete;
	IndexBuffer(IndexBuffer &&other) noexcept;
	IndexBuffer &operator=(IndexBuffer &&other) noexcept;
	~IndexBuffer();
	friend void swap(IndexBuffer &lhs, IndexBuffer &rhs) noexcept;
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const noexcept;
	DXGI_FORMAT getIndexFormat() const noexcept;
	uint32 getIndexBufferSize() const noexcept;
	uint32 getIndexCount() const noexcept;
	static uint32 getIndexStrideByFormat(DXGI_FORMAT format);
	bool isEmpty() const noexcept;
	virtual WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
private:
	uint32                         _indexBufferByteSize;
	DXGI_FORMAT                    _indexFormat;
	std::unique_ptr<DefaultBuffer> _pGPUBuffer;
	WRL::ComPtr<ID3DBlob>          _pCPUBuffer;
};

}