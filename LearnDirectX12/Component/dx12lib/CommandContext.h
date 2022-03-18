#pragma once
#include "dx12libStd.h"
#include "StructConstantBuffer.hpp"
#include <memory>
#include "Math/MathHelper.h"

namespace dx12lib {

using namespace Math;

struct NonCopyable {
	NonCopyable() = default;
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
};

class CommandContext : public NonCopyable {
public:
	template<typename T1, typename T2> requires(std::is_base_of_v<IResource, T1> &&std::is_base_of_v<IResource, T2>)
	void copyResource(std::shared_ptr<T1> &pLhs, std::shared_ptr<T2> &pRhs) {
		this->copyResourceImpl(
			std::static_pointer_cast<IResource>(pLhs),
			std::static_pointer_cast<IResource>(pRhs)
		);
	}
	virtual void copyResourceImpl(std::shared_ptr<IResource> &pLhs, std::shared_ptr<IResource> &pRhs) = 0;

	template<typename T> requires(std::is_base_of_v<IResource, T>)
	void transitionBarrier(std::shared_ptr<T> pBuffer, D3D12_RESOURCE_STATES state, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarrier = false) {
		this->transitionBarrierImpl(
			std::static_pointer_cast<IResource>(pBuffer),
			state,
			subResource,
			flushBarrier
		);
	}
	virtual void transitionBarrierImpl(std::shared_ptr<IResource> pBuffer, D3D12_RESOURCE_STATES state, UINT subResource, bool flushBarrier) = 0;

	template<typename T1, typename T2> requires(std::is_base_of_v<IResource, T1> &&std::is_base_of_v<IResource, T2>)
	void aliasBarrier(std::shared_ptr<T1> pBeforce, std::shared_ptr<T2> pAfter, bool flushBarrier = false) {
		this->aliasBarrierImpl(
			std::static_pointer_cast<IResource>(pBeforce),
			std::static_pointer_cast<IResource>(pAfter),
			flushBarrier
		);
	}
	virtual void aliasBarrierImpl(std::shared_ptr<IResource> pBeforce, std::shared_ptr<IResource> pAfter, bool flushBarrier) = 0;
	virtual ID3D12GraphicsCommandList *getD3DCommandList() const noexcept = 0;
	virtual	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) = 0;
	virtual void flushResourceBarriers() = 0;
	virtual void setConstantBufferView(std::shared_ptr<ConstantBuffer> pConstantBuffer, uint32 rootIndex, uint32 offset = 0) = 0;

	template<typename T> requires(sizeof(T) > 0 && std::is_base_of_v<IShaderSourceResource, T>)
	void setShaderResourceBuffer(std::shared_ptr<T> pResource, uint32 rootIndex, uint32 offset = 0) {
		this->setShaderResourceBufferImpl(
			std::static_pointer_cast<IShaderSourceResource>(pResource),
			rootIndex,
			offset
		);
	}
	virtual void setShaderResourceBufferImpl(std::shared_ptr<IShaderSourceResource> pTexture, uint32 rootIndex, uint32 offset) = 0;
	virtual std::shared_ptr<ConstantBuffer> createConstantBuffer(std::size_t sizeInByte, const void *pData = nullptr) = 0;
};

class GrahpicsContext : public virtual CommandContext {
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
	virtual void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float4 color) = 0;
	virtual void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float colors[4]) = 0;
	virtual void clearDepth(std::shared_ptr<DepthStencilBuffer> pResource, float depth) = 0;
	virtual void clearStencil(std::shared_ptr<DepthStencilBuffer> pResource, UINT stencil) = 0;
	virtual void clearDepthStencil(std::shared_ptr<DepthStencilBuffer> pResource, float depth, UINT stencil) = 0;
	virtual std::shared_ptr<ShaderResourceBuffer> createDDSTextureFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<ShaderResourceBuffer> createDDSTextureFromMemory(const void *pData, std::size_t sizeInByte) = 0;
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

class ComputeContext : public virtual CommandContext {
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