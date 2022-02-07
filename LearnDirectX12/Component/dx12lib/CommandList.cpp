#include "CommandList.h"
#include "Device.h"
#include "FrameResourceQueue.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "ResourceStateTracker.h"
#include "DynamicDescriptorHeap.h"

namespace dx12lib {

ID3D12GraphicsCommandList *CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

void CommandList::setViewports(const D3D12_VIEWPORT &viewport) {
	_pCommandList->RSSetViewports(1, &viewport);
}

void CommandList::setViewprots(const std::vector<D3D12_VIEWPORT> &viewports) {
	_pCommandList->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}

void CommandList::setScissorRects(const D3D12_RECT &rect) {
	_pCommandList->RSSetScissorRects(1, &rect);
}

void CommandList::setScissorRects(const std::vector<D3D12_RECT> &rects) {
	_pCommandList->RSSetScissorRects(static_cast<UINT>(rects.size()), rects.data());
}

void CommandList::setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) {
	assert(pRenderTarget != nullptr);
	
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilViews;
	for (std::size_t i = 0; i < static_cast<std::size_t>(AttachmentPoint::DepthStencil); ++i) {
		auto pTexture = pRenderTarget->getTexture(static_cast<AttachmentPoint>(i));
		if (pTexture == nullptr)
			continue;
		
		ClearFlag clearFlag = pTexture->getClearFlag();
		const auto *pClearValue = pTexture->getClearValue();
		if (clearFlag & ClearFlag::Color && pTexture->checkRTVSupport()) {
			_pCommandList->ClearRenderTargetView(
				pTexture->getRenderTargetView(),
				pClearValue->Color,
				0,
				nullptr
			);
			pTexture->setClearFlag(ClearFlag::None);
		}
		renderTargetViews.push_back(pTexture->getRenderTargetView());
	}
	
	auto pDepthStencilTexture = pRenderTarget->getTexture(AttachmentPoint::DepthStencil);
	if (pDepthStencilTexture != nullptr && pDepthStencilTexture->checkDSVSupport()) {
		auto clearFlag = pDepthStencilTexture->getClearFlag();
		const auto *pClearValue = pDepthStencilTexture->getClearValue();
		D3D12_CLEAR_FLAGS flag = static_cast<D3D12_CLEAR_FLAGS>(0);
		flag |= (clearFlag & ClearFlag::Depth)   ? D3D12_CLEAR_FLAG_DEPTH   : static_cast<D3D12_CLEAR_FLAGS>(0);
		flag |= (clearFlag & ClearFlag::Stencil) ? D3D12_CLEAR_FLAG_STENCIL : static_cast<D3D12_CLEAR_FLAGS>(0);
		_pCommandList->ClearDepthStencilView(
			pDepthStencilTexture->getDepthStencilView(),
			flag,
			pClearValue->DepthStencil.Depth,
			pClearValue->DepthStencil.Stencil,
			0,
			nullptr
		);
		pDepthStencilTexture->setClearFlag(ClearFlag::None);
		depthStencilViews.push_back(pDepthStencilTexture->getDepthStencilView());
	}

	_pCommandList->OMSetRenderTargets(
		static_cast<UINT>(renderTargetViews.size()),
		renderTargetViews.data(),
		static_cast<UINT>(depthStencilViews.size()),
		depthStencilViews.data()
	);
}

void CommandList::flushResourceBarriers() {
	_pResourceStateTracker->flushResourceBarriers(shared_from_this());
}	

void CommandList::transitionBarrier(const IResource &resource, 
	D3D12_RESOURCE_STATES state, UINT subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/, 
	bool flushBarrier /*= false */) 
{
	_pResourceStateTracker->transitionResource(resource, state, subresource);
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::transitionBarrier(const IResource *pResource, 
	D3D12_RESOURCE_STATES state, 
	UINT subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) 
{
	transitionBarrier(*pResource, state, subresource, flushBarrier);
}

void CommandList::transitionBarrier(std::shared_ptr<IResource> pResource, 
	D3D12_RESOURCE_STATES state, 
	UINT subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/, 
	bool flushBarrier /*= false */) 
{
	transitionBarrier(pResource.get(), state, subresource, flushBarrier);
}

void CommandList::aliasBarrier(const IResource *pResourceBeforce /*= nullptr*/, 
	const IResource *pResourceAfter /*= nullptr*/, 
	bool flushBarrier /*= false */) 
{
	_pResourceStateTracker->aliasBarrier(pResourceBeforce, pResourceAfter);
	if (flushBarrier)
		flushResourceBarriers();
}

CommandList::CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem) {
	auto pSharedFrameResourceItem = pFrameResourceItem.lock();
	_cmdListType = pSharedFrameResourceItem->getCommandListType();
	_pDevice = pSharedFrameResourceItem->getDevice();

	auto pDevice = pSharedFrameResourceItem->getDevice();
	auto pd3d12Device = pDevice.lock()->getD3DDevice();
	ThrowIfFailed(pd3d12Device->CreateCommandAllocator(
		_cmdListType,
		IID_PPV_ARGS(&_pCmdListAlloc)
	));
	ThrowIfFailed(pd3d12Device->CreateCommandList(
		0,
		_cmdListType,
		_pCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(&_pCommandList)
	));

	_pResourceStateTracker = std::make_unique<ResourceStateTracker>();

	for (std::size_t i = 0; i < kDynamicDescriptorHeapCount; ++i) {
		_pDynamicDescriptorHeaps[i] = std::make_unique<DynamicDescriptorHeap>(
			_pDevice,
			static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i),
			kDynamicDescriptorPerHeap
		);
	}
}

CommandList::~CommandList() {
}

void CommandList::close() {
	flushResourceBarriers();
	ThrowIfFailed(_pCommandList->Close());
}

void CommandList::close(std::shared_ptr<CommandList> pPendingCmdList) {
	_pResourceStateTracker->flusePendingResourceBarriers(pPendingCmdList);
	flushResourceBarriers();
	_pResourceStateTracker->commitFinalResourceStates();
	ThrowIfFailed(_pCommandList->Close());
}

void CommandList::reset() {
	_pCmdListAlloc->Reset();
	ThrowIfFailed(_pCommandList->Reset(_pCmdListAlloc.Get(), nullptr));

	_pResourceStateTracker->reset();

	for (auto &pDynamicDescriptorHeap : _pDynamicDescriptorHeaps)
		pDynamicDescriptorHeap->reset();
}

}