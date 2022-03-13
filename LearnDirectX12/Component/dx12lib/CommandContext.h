#pragma once
#include "dx12libStd.h"
#include "StructConstantBuffer.hpp"
#include <memory>

namespace dx12lib {

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
};

class CommandContext : public NonCopyable {
public:
	virtual void copyResource(Texture &Dest, Texture &Src) = 0;
	virtual ID3D12GraphicsCommandList *getD3DCommandList() const noexcept = 0;
	virtual	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) = 0;
	virtual void flushResourceBarriers() = 0;
	virtual void setConstantBufferView(std::shared_ptr<ConstantBuffer> pConstantBuffer, uint32 rootIndex, uint32 offset = 0) = 0;
	virtual void setShaderResourceView(std::shared_ptr<Texture> pTexture, uint32 rootIndex, uint32 offset = 0) = 0;
	virtual std::shared_ptr<ConstantBuffer> createConstantBuffer(std::size_t sizeInByte, const void *pData = nullptr) = 0;
};

class GrahpicsContext : public CommandContext {
public:
	virtual void setViewports(const D3D12_VIEWPORT &viewport) = 0;
	virtual void setViewprots(const std::vector<D3D12_VIEWPORT> &viewports) = 0;
	virtual void setScissorRects(const D3D12_RECT &rect) = 0;
	virtual void setScissorRects(const std::vector<D3D12_RECT> &rects) = 0;
	virtual void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) = 0;
	virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const void *pData, std::size_t sizeInByte, std::size_t stride) = 0;
	virtual std::shared_ptr<IndexBuffer> createIndexBuffer(const void *pData, std::size_t sizeInByte, DXGI_FORMAT indexFormat) = 0;
	virtual void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot = 0) = 0;
	virtual void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) = 0;
	virtual void setPipelineStateObject(std::shared_ptr<GraphicsPSO> pPipelineStateObject) = 0;
	virtual void setGrahicsRootSignature(std::shared_ptr<RootSignature> pRootSignature) = 0;
	virtual void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) = 0;
	virtual void setStencilRef(UINT stencilRef) = 0;
	virtual void drawInstanced(uint32 vertCount, uint32 instanceCount, uint32 baseVertexLocation, uint32 startInstanceLocation) = 0;
	virtual void drawIndexdInstanced(uint32 indexCountPerInstance, uint32 instanceCount, uint32 startIndexLocation, uint32 baseVertexLocation, uint32 startInstanceLocation) = 0;
	virtual std::shared_ptr<Texture> createDDSTextureFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<Texture> createDDSTextureFromMemory(const void *pData, std::size_t sizeInByte) = 0;
	template<StructConstantBufferConcept T>
	std::shared_ptr<StructConstantBuffer<T>> createStructConstantBuffer(const T *pData = nullptr) {
		std::shared_ptr<ConstantBuffer> pConstantBuffer = createConstantBuffer(sizeof(T), nullptr);
		return std::make_shared<StructConstantBuffer<T>>(pData, pConstantBuffer);
	}
	template<StructConstantBufferConcept T>
	std::shared_ptr<StructConstantBuffer<T>> createStructConstantBuffer(const T &buff) {
		return this->template createStructConstantBuffer<T>(&buff);
	}
	template<typename T>
	void setStructConstantBuffer(std::shared_ptr<StructConstantBuffer<T>> pStructConstantBuffer,
		uint32 rootIndex,
		uint32 offset = 0) {
		assert(pStructConstantBuffer != nullptr);
		pStructConstantBuffer->updateConstantBuffer();
		setConstantBufferView(pStructConstantBuffer->getConstantBuffer(), rootIndex, offset);
	}
};

class ComputeContext : public CommandContext {
public:
	virtual void dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1) = 0;
	virtual void setComputeRootSignature(std::shared_ptr<RootSignature> pRootSingature) = 0;
	virtual void setPipelineStateObject(std::shared_ptr<ComputePSO> pPipelineStateObject) = 0;
	virtual std::shared_ptr<StructedBuffer> createStructedBuffer(const void *pData, std::size_t sizeInByte) = 0;
	virtual std::shared_ptr<UnorderedAccessBuffer> createUnorderedAccessBuffer(std::size_t sizeInByte) = 0;
	virtual std::shared_ptr<ReadbackBuffer> createReadbackBuffer(std::size_t sizeInByte) = 0;
	virtual void setStructedBuffer(std::shared_ptr<StructedBuffer> pStructedBuffer, uint32 rootIndex, uint32 offset = 0) = 0;
	virtual void setUnorderedAccessBuffer(std::shared_ptr<UnorderedAccessBuffer> pBuffer, uint32 rootIndex, uint32 offset = 0) = 0;
};


}