#pragma once
#include "DefaultBuffer.h"

namespace dx12lib {

class IndexBuffer : public IResource {
protected:
	IndexBuffer();
	IndexBuffer(std::weak_ptr<Device> pDevice, 
		std::shared_ptr<CommandList> pCmdList, 
		const void *pData,
		size_t numElements,
		DXGI_FORMAT format
	);
public:
	IndexBuffer(IndexBuffer &&other) noexcept;
	IndexBuffer &operator=(IndexBuffer &&other) noexcept;
	~IndexBuffer() override;
	friend void swap(IndexBuffer &lhs, IndexBuffer &rhs) noexcept;
	D3D12_INDEX_BUFFER_VIEW getIndexBufferView() const noexcept;
	DXGI_FORMAT getIndexFormat() const noexcept;
	size_t getIndexBufferSize() const noexcept;
	size_t getIndexCount() const noexcept;
	static size_t getIndexStrideByFormat(DXGI_FORMAT format);
	bool isEmpty() const noexcept;
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
private:
	DXGI_FORMAT                    _indexFormat;
	std::unique_ptr<DefaultBuffer> _pDefaultBUffer;
};

}