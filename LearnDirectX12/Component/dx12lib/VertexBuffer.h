#pragma once
#include "dx12libStd.h"
#include "UploadBuffer.h"
#include <memory>

namespace dx12lib {

class VertexBuffer : public IResource {
protected:
	VertexBuffer();
	VertexBuffer(std::weak_ptr<Device> pDevice, std::shared_ptr<CommandList> pCmdList, 
		const void *pData, size_t numElements, size_t stride
	);
public:
	VertexBuffer(const VertexBuffer &) = delete;
	VertexBuffer(VertexBuffer &&other) noexcept;
	VertexBuffer &operator=(VertexBuffer &&other) noexcept;
	~VertexBuffer() override = default;
	friend void swap(VertexBuffer &lhs, VertexBuffer &rhs) noexcept;
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const noexcept;
	size_t getVertexBufferSize() const noexcept;
	size_t getVertexStride() const noexcept;
	size_t getVertexCount() const noexcept;
	bool isEmpty() const noexcept;
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
private:
	size_t _vertexStride;
	std::unique_ptr<DefaultBuffer> _pDefaultBuffer;
};

}