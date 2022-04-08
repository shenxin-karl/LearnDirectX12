#pragma once
#include <memory>
#include "dx12libStd.h"
#include "IResource.h"
#include "Math/MathHelper.h"
#include "MakeObejctTool.hpp"
#include "FRConstantBuffer.hpp"

namespace dx12lib {

using namespace Math;

struct Context {
	Context() = default;
	Context(const Context &) = delete;
	Context &operator=(const Context &) = delete;
	virtual std::weak_ptr<Device> getDevice() const = 0;
	virtual ID3D12GraphicsCommandList *getD3DCommandList() const noexcept = 0;
	virtual ~Context() = default;
};

class CommandContext : public Context {
public:
	virtual std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t sizeInByte, const void *pData = nullptr) = 0;
	virtual std::shared_ptr<StructuredBuffer> createStructuredBuffer(const void *pData, size_t sizeInByte) = 0;
	virtual std::shared_ptr<ShaderResourceBuffer> createDDSTextureFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<ShaderResourceBuffer> createDDSTextureFromMemory(const void *pData, size_t sizeInByte) = 0;
	virtual void setShaderResourceBufferImpl(std::shared_ptr<IShaderSourceResource> pTexture, size_t rootIndex, size_t offset) = 0;
	virtual	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) = 0;

	virtual void setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, size_t rootIndex, size_t offset = 0) = 0;
	virtual void setConstantBufferImpl(std::shared_ptr<IConstantBuffer> pConstantBuffer, size_t rootIndex, size_t offset) = 0;
	virtual void copyResourceImpl(std::shared_ptr<IResource> pDest, std::shared_ptr<IResource> pSrc) = 0;
	virtual void transitionBarrierImpl(std::shared_ptr<IResource> pBuffer, D3D12_RESOURCE_STATES state, UINT subResource, bool flushBarrier) = 0;
	virtual void aliasBarrierImpl(std::shared_ptr<IResource> pBefore, std::shared_ptr<IResource> pAfter, bool flushBarrier) = 0;
	virtual void flushResourceBarriers() = 0;

	template<typename T1, typename T2> requires(std::is_base_of_v<IResource, T1> &&std::is_base_of_v<IResource, T2>)
	void copyResource(std::shared_ptr<T1> pDest, std::shared_ptr<T2> pSrc) {
		this->copyResourceImpl(
			std::static_pointer_cast<IResource>(pDest),
			std::static_pointer_cast<IResource>(pSrc)
		);
	}

	template<typename T> requires(std::is_base_of_v<IResource, T>)
	void transitionBarrier(std::shared_ptr<T> pBuffer, D3D12_RESOURCE_STATES state, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarrier = false) {
		this->transitionBarrierImpl(
			std::static_pointer_cast<IResource>(pBuffer),
			state,
			subResource,
			flushBarrier
		);
	}

	template<typename T1, typename T2> requires(std::is_base_of_v<IResource, T1> &&std::is_base_of_v<IResource, T2>)
	void aliasBarrier(std::shared_ptr<T1> pBefore, std::shared_ptr<T2> pAfter, bool flushBarrier = false) {
		this->aliasBarrierImpl(
			std::static_pointer_cast<IResource>(pBefore),
			std::static_pointer_cast<IResource>(pAfter),
			flushBarrier
		);
	}

	template<typename T> requires(sizeof(T) > 0 && std::is_base_of_v<IShaderSourceResource, T>)
	void setShaderResourceBuffer(std::shared_ptr<T> pResource, size_t rootIndex, size_t offset = 0) {
		this->setShaderResourceBufferImpl(
			std::static_pointer_cast<IShaderSourceResource>(pResource),
			rootIndex,
			offset
		);
	}

	template<typename T>
	std::shared_ptr<FRConstantBuffer<T>> createFRConstantBuffer(const T &data) {
		return this->createFRConstantBuffer(&data);
	}

	template<typename T>
	std::shared_ptr<FRConstantBuffer<T>> createFRConstantBuffer(const T *pData = nullptr) {
		size_t frameIndex = FrameIndexProxy::getConstantFrameIndexRef();
		auto pDevice = getDevice();
		const T &object = (pData != nullptr) ? *pData : T{};
		return std::make_shared<dx12libTool::MakeFRConstantBuffer<T>>(pDevice, object);
	}

	template<typename T>
	void setConstantBuffer(std::shared_ptr<FRConstantBuffer<T>> pConstantBuffer, size_t rootIndex, size_t offset = 0) {
		this->setConstantBufferImpl(
			std::static_pointer_cast<IConstantBuffer>(pConstantBuffer),
			rootIndex,
			offset
		);
	}
};

class GraphicsContext : public virtual CommandContext {
public:
	virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const void *pData, size_t sizeInByte, size_t stride) = 0;
	virtual std::shared_ptr<IndexBuffer> createIndexBuffer(const void *pData, size_t sizeInByte, DXGI_FORMAT indexFormat) = 0;

	virtual void setViewport(const D3D12_VIEWPORT &viewport) = 0;
	virtual void setScissorRect(const D3D12_RECT &rect) = 0;
	virtual void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) = 0;
	virtual void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot = 0) = 0;
	virtual void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) = 0;
	virtual void setGraphicsPSO(std::shared_ptr<GraphicsPSO> pPipelineStateObject) = 0;
	virtual void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) = 0;
	virtual void setStencilRef(UINT stencilRef) = 0;
	virtual void setGraphics32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset = 0) = 0;

	virtual void drawInstanced(size_t vertCount, size_t instanceCount, size_t baseVertexLocation, size_t startInstanceLocation) = 0;
	virtual void drawIndexedInstanced(size_t indexCountPerInstance, size_t instanceCount, size_t startIndexLocation, size_t baseVertexLocation, size_t startInstanceLocation) = 0;

	virtual void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float4 color) = 0;
	virtual void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float colors[4]) = 0;
	virtual void clearDepth(std::shared_ptr<DepthStencilBuffer> pResource, float depth) = 0;
	virtual void clearStencil(std::shared_ptr<DepthStencilBuffer> pResource, UINT stencil) = 0;
	virtual void clearDepthStencil(std::shared_ptr<DepthStencilBuffer> pResource, float depth, UINT stencil) = 0;
};

class ComputeContext : public virtual CommandContext {
public:
	virtual std::shared_ptr<UnorderedAccessBuffer> createUnorderedAccessBuffer(size_t width, size_t height, DXGI_FORMAT format) = 0;
	virtual std::shared_ptr<ReadBackBuffer> createReadBackBuffer(size_t sizeInByte) = 0;

	virtual void setComputePSO(std::shared_ptr<ComputePSO> pPipelineStateObject) = 0;
	virtual void setUnorderedAccessBuffer(std::shared_ptr<UnorderedAccessBuffer> pBuffer, size_t rootIndex, size_t offset = 0) = 0;
	virtual void setCompute32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset = 0) = 0;

	virtual void dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1) = 0;
	virtual void readBack(std::shared_ptr<ReadBackBuffer> pReadBackBuffer) = 0;
};

class DirectContext : public GraphicsContext, public ComputeContext {
};

}