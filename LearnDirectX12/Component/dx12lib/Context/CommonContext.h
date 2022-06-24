#pragma once
#include <memory>
#include <Math/MathStd.hpp>
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Tool/MakeObejctTool.hpp>
#include <dx12lib/Buffer/FRConstantBuffer.hpp>

namespace dx12lib {

using namespace Math;


interface IContext {
	IContext() = default;
	IContext(const IContext &) = delete;
	IContext &operator=(const IContext &) = delete;
	virtual std::weak_ptr<Device> getDevice() const = 0;
	virtual ID3D12GraphicsCommandList *getD3DCommandList() const noexcept = 0;
	virtual std::shared_ptr<CommandList> getCommandList() noexcept = 0;
	virtual ~IContext() = default;
};

interface ICommonContext : IContext {
	virtual void trackResource(std::shared_ptr<IResource> &&pResource) = 0;
	virtual std::shared_ptr<SamplerTexture2D> createDDSTexture2DFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<SamplerTexture2D> createDDSTexture2DFromMemory(const void *pData, size_t sizeInByte) = 0;
	virtual std::shared_ptr<SamplerTexture2DArray> createDDSTexture2DArrayFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<SamplerTexture2DArray> createDDSTexture2DArrayFromMemory(const void *pData, size_t sizeInByte) = 0;
	virtual std::shared_ptr<SamplerTextureCube> createDDSTextureCubeFromFile(const std::wstring &fileName) = 0;
	virtual std::shared_ptr<SamplerTextureCube> createDDSTextureCubeFromMemory(const void *pData, size_t sizeInByte) = 0;
	virtual std::shared_ptr<IShaderResource> createTextureFromFile(const std::wstring &fileName, bool sRGB = false) = 0;
	virtual	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) = 0;
	virtual void setConstantBufferView(const ShaderRegister &sr, const ConstantBufferView &crv) = 0;
	virtual void setShaderResourceView(const ShaderRegister &sr, const ShaderResourceView &srv) = 0;
	virtual void readBack(std::shared_ptr<ReadBackBuffer> pReadBackBuffer) = 0;

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

	/////////////////////////////////// ConstantBuffer //////////////////////////////////
#if 1
	template<typename T>
	std::shared_ptr<ConstantBuffer> createConstantBuffer() {
		return std::make_shared<dx12libTool::MakeConstantBuffer>(
			getDevice(),
			nullptr,
			sizeof(T)
		);
	}

	template<typename...Args>
	std::shared_ptr<ConstantBuffer> createConstantBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeConstantBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

 	template<typename T, typename...Args>
	std::shared_ptr<FRConstantBuffer<T>> createFRConstantBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeFRConstantBuffer<T>>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<FRConstantBuffer<RawData>> createFRRawConstantBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeFRRawConstantBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename T> requires(std::is_base_of_v<IConstantBuffer, T>)
	void setConstantBuffer(const ShaderRegister &sr, std::shared_ptr<T> pBuffer) {
		auto pConstantBuffer = std::static_pointer_cast<IConstantBuffer>(pBuffer);
		this->setConstantBufferView(sr, pConstantBuffer->getCBV());
	}
#endif
	/////////////////////////////////// SRStructuredBuffer //////////////////////////////////
#if 1
	template<typename... Args>
	std::shared_ptr<SRStructuredBuffer> createStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeSRStructuredBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename T, typename...Args>
	std::shared_ptr<FRSRStructuredBuffer<T>> createFRStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeFRSRStructuredBuffer<T>>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<FRSRStructuredBuffer<RawData>> createFRRawStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeFRRawSRStructuredBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename T> requires(std::is_base_of_v<IStructuredBuffer, T>)
	void setStructuredBuffer(const ShaderRegister &sr, std::shared_ptr<T> pBuffer) {
		auto pStructuredBuffer = std::static_pointer_cast<IStructuredBuffer>(pBuffer);
		this->setShaderResourceView(sr, pStructuredBuffer->getSRV());
	}
#endif
	/////////////////////////////////// RenderTarget //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<RenderTarget2D> createRenderTarget2D(Args&&...args) {
		return std::make_shared<dx12libTool::MakeRenderTarget2D>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<RenderTarget2DArray> createRenderTarget2DArray(Args&&...args) {
		return std::make_shared<dx12libTool::MakeRenderTarget2DArray>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<RenderTargetCube> createRenderTargetCube(Args&&...args) {
		return std::make_shared<dx12libTool::MakeRenderTargetCube>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}
#endif
	/////////////////////////////////// DepthBuffer //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<DepthStencil2D> createDepthStencil2D(Args&&... args) {
		return std::make_shared<dx12libTool::MakeDepthStencil2D>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}
#endif
	/////////////////////////////////// ReadBackBuffer //////////////////////////////////
#if 1
	template<typename... Args>
	std::shared_ptr<ReadBackBuffer> createReadBackBuffer(Args&&... args) {
		return std::make_shared<dx12libTool::MakeReadBackBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}
#endif
};

interface IGraphicsContext : virtual ICommonContext {
	virtual std::shared_ptr<VertexBuffer> createVertexBuffer(const void *pData, size_t numElements, size_t stride) = 0;
	virtual std::shared_ptr<IndexBuffer> createIndexBuffer(const void *pData, size_t numElements, DXGI_FORMAT indexFormat) = 0;

	virtual void setViewport(const D3D12_VIEWPORT &viewport) = 0;
	virtual void setScissorRect(const D3D12_RECT &rect) = 0;
	virtual void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot = 0) = 0;
	virtual void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) = 0;
	virtual void setGraphicsPSO(std::shared_ptr<GraphicsPSO> pPipelineStateObject) = 0;
	virtual void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) = 0;
	virtual void setStencilRef(UINT stencilRef) = 0;
	virtual void setGraphics32BitConstants(const ShaderRegister &sr, size_t numConstants, const void *pData, size_t destOffset = 0) = 0;
	virtual void setRenderTarget(const RenderTargetView &rtv, const DepthStencilView &dsv) = 0;
	virtual void setRenderTargets(const std::vector<RenderTargetView> &rtvs, const DepthStencilView &dsv) = 0;

	virtual void drawInstanced(size_t vertCount, size_t instanceCount, size_t baseVertexLocation, size_t startInstanceLocation = 0) = 0;
	virtual void drawIndexedInstanced(size_t indexCountPerInstance, size_t instanceCount, size_t startIndexLocation, size_t baseVertexLocation, size_t startInstanceLocation) = 0;

	virtual void clearColor(std::shared_ptr<RenderTarget2D> pResource, float4 color) = 0;
	virtual void clearColor(std::shared_ptr<RenderTarget2D> pResource, float colors[4]) = 0;
	virtual void clearDepth(std::shared_ptr<DepthStencil2D> pResource, float depth) = 0;
	virtual void clearStencil(std::shared_ptr<DepthStencil2D> pResource, UINT stencil) = 0;
	virtual void clearDepthStencil(std::shared_ptr<DepthStencil2D> pResource, float depth, UINT stencil) = 0;
};

interface IComputeContext : virtual ICommonContext {
	virtual void setComputePSO(std::shared_ptr<ComputePSO> pPipelineStateObject) = 0;
	virtual void setUnorderedAccessView(const ShaderRegister &sr, const UnorderedAccessView &uav) = 0;
	virtual void setCompute32BitConstants(const ShaderRegister &sr, size_t numConstants, const void *pData, size_t destOffset = 0) = 0;
	virtual void dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1) = 0;

	/////////////////////////////////// UnorderedAccess //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<UnorderedAccess2D> createUnorderedAccess2D(Args&&...args) {
		return std::make_shared<dx12libTool::MakeUnorderedAccess2D>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<UnorderedAccess2DArray> createUnorderedAccess2DArray(Args&&...args) {
		return std::make_shared<dx12libTool::MakeUnorderedAccess2DArray>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}

	template<typename...Args>
	std::shared_ptr<UnorderedAccessCube> createUnorderedAccessCube(Args&&...args) {
		return std::make_shared<dx12libTool::MakeUnorderedAccessCube>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}
#endif
	/////////////////////////////////// ConsumeStructured //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<ConsumeStructuredBuffer> createConsumeStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeConsumeStructuredBuffer>(
			getDevice(),
			getCommandList(),
			std::forward<Args>(args)...
		);
	}
#endif
	/////////////////////////////////// AppendStructured //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<AppendStructuredBuffer> createAppendStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeAppendStructuredBuffer>(
			getDevice(),
			std::forward<Args>(args)...
		);
	}
#endif
	/////////////////////////////////// UAStructuredBuffer //////////////////////////////////
#if 1
	template<typename...Args>
	std::shared_ptr<UAStructuredBuffer> createUAStructuredBuffer(Args&&...args) {
		return std::make_shared<dx12libTool::MakeUAStructuredBuffer>(
			getDevice(),
			getCommandList(),
			std::forward<Args>(args)...
		);
	}
#endif
};

interface IDirectContext : IGraphicsContext, IComputeContext {
};

}