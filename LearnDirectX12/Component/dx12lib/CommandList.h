#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"
#include <memory>

namespace dx12lib {

class CommandList : public std::enable_shared_from_this<CommandList> {
public:
	CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem);
	~CommandList();
	ID3D12GraphicsCommandList *getD3DCommandList() const noexcept;
	void setViewports(const D3D12_VIEWPORT &viewport);
	void setViewprots(const std::vector<D3D12_VIEWPORT> &viewports);
	void setScissorRects(const D3D12_RECT &rect);
	void setScissorRects(const std::vector<D3D12_RECT> &rects);
	void setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget);
	void flushResourceBarriers();
	void setPipelineStateObject(std::shared_ptr<PipelineStateObject> pPso);
// barrier
	void transitionBarrier(const IResource &resource, 
		D3D12_RESOURCE_STATES state,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void transitionBarrier(const IResource *pResource, 
		D3D12_RESOURCE_STATES state, 
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void transitionBarrier(std::shared_ptr<IResource> pResource,
		D3D12_RESOURCE_STATES state,
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
		bool flushBarrier = false
	);
	void aliasBarrier(const IResource *pResourceBeforce = nullptr,
		const IResource *pResourceAfter = nullptr,
		bool flushBarrier = false
	);
// buffer
	std::shared_ptr<VertexBuffer> createVertexBuffer(
		const void *pData,
		uint32 sizeInByte,
		uint32 stride,
		uint32 slot = 0
	);

	std::shared_ptr<IndexBuffer> createIndexBuffer(
		const void *pData,
		uint32 sizeInByte,
		DXGI_FORMAT indexFormat
	);

	std::shared_ptr<ConstantBuffer> createConstantBuffer(uint32 sizeInByte, const void *pData = nullptr);
	
	void setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer);
	void setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer);
	void setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, uint32 rootIndex, uint32 offset = 0);
private:
	friend class CommandQueue;
	friend class FrameResourceItem;
	void close();
	void close(CommandListProxy pPendingCmdList);
	void reset();
private:
	D3D12_COMMAND_LIST_TYPE                 _cmdListType;
	std::weak_ptr<Device>                   _pDevice;
	WRL::ComPtr<ID3D12GraphicsCommandList>  _pCommandList;
	WRL::ComPtr<ID3D12CommandAllocator>     _pCmdListAlloc;
	std::unique_ptr<ResourceStateTracker>   _pResourceStateTracker;
	std::unique_ptr<DynamicDescriptorHeap>  _pDynamicDescriptorHeaps[kDynamicDescriptorHeapCount];
};

}