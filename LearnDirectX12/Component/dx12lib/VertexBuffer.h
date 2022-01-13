#pragma once
#include "dx12libCommon.h"
#include "UploadBuffer.h"
#include "DefaultBuffer.h"
#include <memory>

namespace dx12lib {

class VertexBuffer {
public:
	VertexBuffer();
	VertexBuffer(ID3D12Device *pDevice, ID3D12GraphicsCommandList *pCmdList, void *pData, uint32 sizeInByte, uint32 stride);
	VertexBuffer(const VertexBuffer &) = delete;
	VertexBuffer(VertexBuffer &&other) noexcept;
	VertexBuffer &operator=(VertexBuffer &&other) noexcept;
	~VertexBuffer() = default;
	friend void swap(VertexBuffer &lhs, VertexBuffer &rhs) noexcept;
	D3D12_VERTEX_BUFFER_VIEW getVertexBufferView() const noexcept;
private:
	uint32  _bufferByteSize;
	uint32  _vertexStride;
	WRL::ComPtr<ID3DBlob> _pCPUBuffer;
	std::unique_ptr<DefaultBuffer> _pGPUBuffer;
};

}