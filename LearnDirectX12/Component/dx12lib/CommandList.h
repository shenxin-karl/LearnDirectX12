#pragma once
#include "dx12libStd.h"
#include "ContextProxy.hpp"
#include "CommonContext.h"
#include <memory>

namespace dx12lib {

class CommandList : public std::enable_shared_from_this<CommandList>, public DirectContext {
protected:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
public:
	~CommandList();
/// Context api
	std::weak_ptr<Device> getDevice() const override;
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept override;

/// CommonContext api
	std::shared_ptr<ConstantBuffer> createConstantBuffer(size_t sizeInByte, const void *pData) override;
	std::shared_ptr<FRConstantBuffer<RawData>> createFRRawConstantBuffer(size_t sizeInByte, const void *pData) override;
	std::shared_ptr<StructuredBuffer> createStructuredBuffer(const void *pData, size_t numElements, size_t stride) override;
	std::shared_ptr<FRStructuredBuffer<RawData>> createFRRawStructuredBuffer(const void *pData, size_t numElements, size_t stride) override;
	std::shared_ptr<Texture2D> createDDSTexture2DFromFile(const std::wstring &fileName) override;
	std::shared_ptr<Texture2D> createDDSTexture2DFromMemory(const void *pData, size_t sizeInByte) override;
	std::shared_ptr<ShaderResourceBuffer> createDDSTextureCubeFromFile(const std::wstring &fileName) override;
	std::shared_ptr<ShaderResourceBuffer> createDDSTextureCubeFromMemory(const void *pData, size_t sizeInByte) override;

	void setShaderResourceBufferImpl(std::shared_ptr<IShaderSourceResource> pTexture, size_t rootIndex, size_t offset) override;
	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) override;
	void setConstantBufferView(const ConstantBufferView &cbv, size_t rootIndex, size_t offset) override;
	void setShaderResourceView(const ShaderResourceView &srv, size_t rootIndex, size_t offset) override;

	void setStructuredBuffer(std::shared_ptr<StructuredBuffer> pStructuredBuffer, size_t rootIndex, size_t offset) override;
	void setStructuredBufferImpl(std::shared_ptr<IStructuredBuffer> pStructuredBuffer, size_t rootIndex, size_t offset) override;
	void setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, size_t rootIndex, size_t offset) override;
	void setConstantBufferImpl(std::shared_ptr<IConstantBuffer> pConstantBuffer, size_t rootIndex, size_t offset) override;
	void copyResourceImpl(std::shared_ptr<IResource> pDest, std::shared_ptr<IResource> pSrc) override;
	void transitionBarrierImpl(std::shared_ptr<IResource> pBuffer, D3D12_RESOURCE_STATES state, UINT subResource, bool flushBarrier) override;
	void aliasBarrierImpl(std::shared_ptr<IResource> pBeforce, std::shared_ptr<IResource> pAfter, bool flushBarrier) override;
	void flushResourceBarriers() override;

/// GraphicsContext api
	std::shared_ptr<VertexBuffer> createVertexBuffer(const void *pData, size_t numElements, size_t stride) override;
	std::shared_ptr<IndexBuffer> createIndexBuffer(const void *pData, size_t numElements, DXGI_FORMAT indexFormat) override;

	void setViewport(const D3D12_VIEWPORT &viewport) override;
	void setScissorRect(const D3D12_RECT &rect) override;
	void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) override;
	void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot) override;
	void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) override;
	void setGraphicsPSO(std::shared_ptr<GraphicsPSO> pPipelineStateObject) override;
	void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) override;
	void setStencilRef(UINT stencilRef) override;
	void setGraphics32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset) override;

	void drawInstanced(size_t vertCount, size_t instanceCount, size_t baseVertexLocation, size_t startInstanceLocation) override;
	void drawIndexedInstanced(size_t indexCountPerInstance, size_t instanceCount, size_t startIndexLocation, size_t baseVertexLocation, size_t startInstanceLocation) override;

	void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float4 color) override;
	void clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float colors[4]) override;
	void clearDepth(std::shared_ptr<DepthStencilBuffer> pResource, float depth) override;
	void clearStencil(std::shared_ptr<DepthStencilBuffer> pResource, UINT stencil) override;
	void clearDepthStencil(std::shared_ptr<DepthStencilBuffer> pResource, float depth, UINT stencil) override;
/// ComputeContext api 
	std::shared_ptr<UnorderedAccessBuffer> createUnorderedAccessBuffer(size_t width, size_t height, DXGI_FORMAT format) override;
	std::shared_ptr<ReadBackBuffer> createReadBackBuffer(size_t sizeInByte) override;

	void setComputePSO(std::shared_ptr<ComputePSO> pPipelineStateObject) override;
	void setUnorderedAccessBuffer(std::shared_ptr<UnorderedAccessBuffer> pBuffer, size_t rootIndex, size_t offset) override;
	void setCompute32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset) override;

	void dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) override;
	void readBack(std::shared_ptr<ReadBackBuffer> pReadBackBuffer) override;
private:
	friend class CommandQueue;
	friend class FrameResourceItem;
	using ReadBackBufferPool = std::vector<std::shared_ptr<ReadBackBuffer>>;
	void setGraphicsRootSignature(std::shared_ptr<RootSignature> pRootSignature);
	void setComputeRootSignature(std::shared_ptr<RootSignature> pRootSignature);
	void close();
	void close(std::shared_ptr<CommandList> pPendingCmdList);
	void reset();
	using SetRootSignatureFunc = std::function<void(ID3D12GraphicsCommandList *, ID3D12RootSignature *)>;
	void setRootSignature(std::shared_ptr<RootSignature> pRootSignature, const SetRootSignatureFunc &setFunc);
	void bindDescriptorHeaps();
private:
	D3D12_COMMAND_LIST_TYPE                _cmdListType;
	std::weak_ptr<Device>                  _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList> _pCommandList;
	WRL::ComPtr<ID3D12CommandAllocator>    _pCmdListAlloc;
	std::unique_ptr<ResourceStateTracker>  _pResourceStateTracker;
	std::unique_ptr<DynamicDescriptorHeap> _pDynamicDescriptorHeaps[kDynamicDescriptorHeapCount];
	ReadBackBufferPool                     _pReadBackBuffers;
private:
	struct CommandListState {
		PSO           *pPSO;
		RootSignature *pRootSignature;
		VertexBuffer  *pVertexBuffers[kVertexBufferSlotCount];
		IResource     *pRTBuffers[AttachmentPoint::NumAttachmentPoints];
		IResource     *pDepthStencil;
		IndexBuffer   *pIndexBuffer;
		RenderTarget  *pRenderTarget;
		bool           isSetViewport;
		bool           isSetScissorRect;
		UINT           stencilRef;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ID3D12DescriptorHeap *pDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	public:
		bool debugCheckDraw() const;
		bool debugCheckDrawIndex() const;
		bool checkVertexBuffer() const;
		bool checkTextures() const;
		bool debugCheckSet32BitConstants(size_t rootIndex, size_t numConstants) const;
		void setRenderTarget(RenderTarget *pRenderTarget);
	};
	CommandListState _currentGPUState;
};

}