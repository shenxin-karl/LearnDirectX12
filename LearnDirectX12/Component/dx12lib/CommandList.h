#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"
#include "StructConstantBuffer.hpp"
#include "CommandContext.h"
#include <memory>

namespace dx12lib {

class CommandList : public std::enable_shared_from_this<CommandList>, public GrahpicsContext, public ComputeContext  {
protected:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
public:
	~CommandList();
	void copyResource(Texture &dest, Texture &src) override;
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept override;
/// viewport scissorRect
	void setViewports(const D3D12_VIEWPORT &viewport) override;
	void setViewprots(const std::vector<D3D12_VIEWPORT> &viewports) override;
	void setScissorRects(const D3D12_RECT &rect) override;
	void setScissorRects(const std::vector<D3D12_RECT> &rects) override;

	void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) override;
	void flushResourceBarriers() override;
/// resource barrier
	void transitionBarrier(const IResource &resource, 
		D3D12_RESOURCE_STATES state,
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void transitionBarrier(const IResource *pResource, 
		D3D12_RESOURCE_STATES state, 
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void transitionBarrier(std::shared_ptr<IResource> pResource,
		D3D12_RESOURCE_STATES state,
		UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void aliasBarrier(const IResource *pResourceBefore = nullptr,
		const IResource *pResourceAfter = nullptr,
		bool flushBarrier = false
	);
/// create gpu buffer
	std::shared_ptr<VertexBuffer> createVertexBuffer(
		const void *pData,
		std::size_t sizeInByte,
		std::size_t stride
	) override;

	std::shared_ptr<IndexBuffer> createIndexBuffer(
		const void *pData,
		std::size_t sizeInByte,
		DXGI_FORMAT indexFormat
	) override;

	std::shared_ptr<ConstantBuffer> createConstantBuffer(std::size_t sizeInByte, const void *pData) override;

	
/// bind gpu buffer
	void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot = 0) override;
	void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) override;
	void setConstantBufferView(std::shared_ptr<ConstantBuffer> pConstantBuffer, 
		uint32 rootIndex, 
		uint32 offset = 0
	) override;
	void setPipelineStateObject(std::shared_ptr<GraphicsPSO> pPipelineStateObject) override;
	void setGrahicsRootSignature(std::shared_ptr<RootSignature> pRootSignature) override;
	void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) override;
	void setShaderResourceView(std::shared_ptr<Texture> pTexture, uint32 rootIndex, uint32 offset = 0) override;
	void setStencilRef(UINT stencilRef) override;

/// draw function
	void drawInstanced(uint32 vertCount, 
		uint32 instanceCount, 
		uint32 baseVertexLocation,
		uint32 startInstanceLocation
	) override;
	
	void drawIndexdInstanced(uint32 indexCountPerInstance, 
		uint32 instanceCount, 
		uint32 startIndexLocation, 
		uint32 baseVertexLocation,
		uint32 startInstanceLocation
	) override;
/// create dds texture
	std::shared_ptr<Texture> createDDSTextureFromFile(const std::wstring &fileName) override;
	std::shared_ptr<Texture> createDDSTextureFromMemory(const void *pData, std::size_t sizeInByte) override;
/// bind descriptor
	void setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, WRL::ComPtr<ID3D12DescriptorHeap> pHeap) override;

/// compute commandlist
	void dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1) override;
	void setComputeRootSignature(std::shared_ptr<RootSignature> pRootSignature) override;
	void setPipelineStateObject(std::shared_ptr<ComputePSO> pPipelineStateObject) override;
private:
	friend class CommandQueue;
	friend class FrameResourceItem;
	void close();
	void close(CommandListProxy pPendingCmdList);
	void reset();

	using SetRootSignatureFunc = std::function<void(ID3D12GraphicsCommandList *, ID3D12RootSignature *)>;
	void setRootSignature(std::shared_ptr<RootSignature> pRootSignature, const SetRootSignatureFunc &setFunc);
	void bindDescriptorHeaps();
private:
	D3D12_COMMAND_LIST_TYPE                 _cmdListType;
	std::weak_ptr<Device>                   _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
	WRL::ComPtr<ID3D12CommandAllocator>     _pCmdListAlloc;
	std::unique_ptr<ResourceStateTracker>   _pResourceStateTracker;
	std::unique_ptr<DynamicDescriptorHeap>  _pDynamicDescriptorHeaps[kDynamicDescriptorHeapCount];
private:
	struct CommandListState {
		PSO           *pPSO;
		RootSignature *pRootSignature;
		VertexBuffer  *pVertexBuffers[kVertexBufferSlotCount];
		Texture       *pTextures[AttachmentPoint::NumAttachmentPoints];
		IndexBuffer   *pIndexBuffer;
		RenderTarget  *pRenderTarget;
		bool           isSetViewprot;
		bool           isSetScissorRect;
		UINT           stencilRef;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ID3D12DescriptorHeap *pDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	public:
		bool debugCheckDraw() const;
		bool debugCheckDrawIndex() const;
		bool checkVertexBuffer() const;
		bool checkTextures() const;
		void setRenderTarget(RenderTarget *pRenderTarget);
	};
	CommandListState _currentGPUState;
};

}