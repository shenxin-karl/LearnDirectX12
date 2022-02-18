#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"
#include "StructConstantBuffer.hpp"
#include <memory>

namespace dx12lib {

class CommandList : public std::enable_shared_from_this<CommandList> {
public:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	~CommandList();
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
/// viewport scissorRect
	void setViewports(const D3D12_VIEWPORT &viewport);
	void setViewprots(const std::vector<D3D12_VIEWPORT> &viewports);
	void setScissorRects(const D3D12_RECT &rect);
	void setScissorRects(const std::vector<D3D12_RECT> &rects);

	void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget);
	void flushResourceBarriers();
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
	);

	std::shared_ptr<IndexBuffer> createIndexBuffer(
		const void *pData,
		std::size_t sizeInByte,
		DXGI_FORMAT indexFormat
	);

	std::shared_ptr<ConstantBuffer> createConstantBuffer(std::size_t sizeInByte, const void *pData = nullptr);

	template<StructConstantBufferConcept T>
	std::shared_ptr<StructConstantBuffer<T>> createStructConstantBuffer(const T *pData = nullptr) {
		std::shared_ptr<ConstantBuffer> pConstantBuffer = createConstantBuffer(sizeof(T), nullptr);
		return std::make_shared<StructConstantBuffer<T>>(pData, pConstantBuffer);
	}

	template<StructConstantBufferConcept T>
	std::shared_ptr<StructConstantBuffer<T>> createStructConstantBuffer(const T &buff) {
		return this->template createStructConstantBuffer<T>(&buff);
	}
	
/// bind gpu buffer
	void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot = 0);
	void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer);
	void setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, uint32 rootIndex, uint32 offset = 0);
	void setPipelineStateObject(std::shared_ptr<GraphicsPSO> pPipelineStateObject);
	void setGrahicsRootSignature(std::shared_ptr<RootSignature> pRootSignature);
	void setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology);

	template<typename T>
	void setStructConstantBuffer(std::shared_ptr<StructConstantBuffer<T>> pStructConstantBuffer,
		uint32 rootIndex,
		uint32 offset = 0) 
	{
		assert(pStructConstantBuffer != nullptr);
		pStructConstantBuffer->updateConstantBuffer();
		setConstantBuffer(pStructConstantBuffer->getConstantBuffer(), rootIndex, offset);
	}
/// draw function
	void drawInstanced(uint32 vertCount, 
		uint32 instanceCount, 
		uint32 startVertex, 
		uint32 startInstance
	);
	
	void drawIndexdInstanced(uint32 indexCountPerInstance, 
		uint32 instanceCount, 
		uint32 startIndexLocation, 
		uint32 startVertexLocation, 
		uint32 startInstanceLocation
	);
private:
	friend class CommandQueue;
	friend class FrameResourceItem;
	void close();
	void close(CommandListProxy pPendingCmdList);
	void reset();

	using SetRootSignatureFunc = std::function<void(ID3D12GraphicsCommandList *, ID3D12RootSignature *)>;
	void setRootSignature(std::shared_ptr<RootSignature> pRootSignature, const SetRootSignatureFunc &setFunc);
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