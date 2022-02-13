#include "CommandList.h"
#include "Device.h"
#include "FrameResourceQueue.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "ResourceStateTracker.h"
#include "DynamicDescriptorHeap.h"
#include "PipelineStateObject.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "CommandQueue.h"

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
	D3D12_RESOURCE_STATES state, UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) 
{
	_pResourceStateTracker->transitionResource(resource, state, subResource);
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::transitionBarrier(const IResource *pResource, 
	D3D12_RESOURCE_STATES state, 
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) 
{
	transitionBarrier(*pResource, state, subResource, flushBarrier);
}

void CommandList::transitionBarrier(std::shared_ptr<IResource> pResource, 
	D3D12_RESOURCE_STATES state, 
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/, 
	bool flushBarrier /*= false */) 
{
	transitionBarrier(pResource.get(), state, subResource, flushBarrier);
}

void CommandList::aliasBarrier(const IResource *pResourceBefore /*= nullptr*/,
	const IResource *pResourceAfter /*= nullptr*/, 
	bool flushBarrier /*= false */) 
{
	_pResourceStateTracker->aliasBarrier(pResourceBefore, pResourceAfter);
	if (flushBarrier)
		flushResourceBarriers();
}

std::shared_ptr<VertexBuffer> 
CommandList::createVertexBuffer(const void *pData, std::size_t sizeInByte, std::size_t stride) {
	return std::make_shared<VertexBuffer>(_pDevice, 
		shared_from_this(), 
		pData, 
		uint32(sizeInByte), 
		uint32(stride)
	);
}

std::shared_ptr<IndexBuffer> 
CommandList::createIndexBuffer(const void *pData, std::size_t sizeInByte, DXGI_FORMAT indexFormat) {
	return std::make_shared<IndexBuffer>(_pDevice, 
		shared_from_this(), 
		pData, 
		uint32(sizeInByte),
		indexFormat
	);
}

std::shared_ptr<ConstantBuffer> 
CommandList::createConstantBuffer(std::size_t sizeInByte, const void *pData) {
	auto pSharedDevice = _pDevice.lock();
	auto queueType = toCommandQueueType(_cmdListType);
	auto *pFrameResourceQueue = pSharedDevice->getCommandQueue(queueType)->getFrameResourceQueue();
	ConstantBufferDesc desc = {
		_pDevice,
		pFrameResourceQueue->getCurrentFrameResourceIndexRef(),
		pFrameResourceQueue->getMaxFrameResourceCount(),
		uint32(sizeInByte),
		pData
	};
	return std::make_shared<ConstantBuffer>(desc);
}

void CommandList::setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot /*= 0 */) {
	assert(pVertBuffer != nullptr);
	_pCommandList->IASetVertexBuffers(
		slot,
		1,
		RVPtr(pVertBuffer->getVertexBufferView())
	);
}

void CommandList::setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) {
	assert(pIndexBuffer != nullptr);
	_pCommandList->IASetIndexBuffer(RVPtr(pIndexBuffer->getIndexBufferView()));
}

void CommandList::setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, uint32 rootIndex, uint32 offset){
	assert(pConstantBuffer != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex, 
		offset, 
		1, 
		pConstantBuffer->getConstantBufferView()
	);
}

void CommandList::setPipelineStateObject(std::shared_ptr<PSO> pPipelineStateObject) {
	assert(pPipelineStateObject != nullptr);
	if (_pCurrentPSO == pPipelineStateObject)
		return;

	setRootSignature(pPipelineStateObject->getRootSignature());
	_pCurrentPSO = pPipelineStateObject;
	_pCommandList->SetPipelineState(pPipelineStateObject->getPipelineStateObject().Get());
}

void CommandList::setRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	if (_pCurrentRootSignature != pRootSignature) {
		_pCurrentRootSignature = pRootSignature;
		for (auto &pHeap : _pDynamicDescriptorHeaps)
			pHeap->parseRootSignature(_pCurrentRootSignature);
	}
}

void CommandList::draw(uint32 vertCount, 
	uint32 instanceCount, 
	uint32 startVertex, 
	uint32 startInstance) 
{
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawInstanced(vertCount, instanceCount, startVertex, startInstance);
}

void CommandList::drawIndex(uint32 indexCountPerInstance, 
	uint32 instanceCount, 
	uint32 startIndexLocation, 
	uint32 startVertexLocation, 
	uint32 startInstanceLocation) 
{
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawIndexedInstanced(indexCountPerInstance, 
		instanceCount, 
		startIndexLocation, 
		startVertexLocation, 
		startIndexLocation
	);
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
			static_cast<uint32>(kDynamicDescriptorPerHeap)
		);
	}
}

CommandList::~CommandList() {
}

void CommandList::close() {
	flushResourceBarriers();
	ThrowIfFailed(_pCommandList->Close());
}

void CommandList::close(CommandListProxy pPendingCmdList) {
	_pResourceStateTracker->flusePendingResourceBarriers(pPendingCmdList->shared_from_this());
	flushResourceBarriers();
	_pResourceStateTracker->commitFinalResourceStates();
	ThrowIfFailed(_pCommandList->Close());
}

void CommandList::reset() {
	_pCmdListAlloc->Reset();
	ThrowIfFailed(_pCommandList->Reset(_pCmdListAlloc.Get(), nullptr));
	_pResourceStateTracker->reset();
	_pCurrentPSO = nullptr;
	_pCurrentRootSignature = nullptr;
	for (auto &pDynamicDescriptorHeap : _pDynamicDescriptorHeaps)
		pDynamicDescriptorHeap->reset();
}

}