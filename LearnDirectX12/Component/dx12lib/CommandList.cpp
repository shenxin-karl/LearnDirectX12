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
#include "RootSignature.h"
#include "MakeObejctTool.hpp"
#include "DefaultBuffer.h"
#include "DDSTextureLoader.h"
#include "RenderTargetBuffer.h"
#include "DepthStencilBuffer.h"
#include <iostream>

#if defined(_DEBUG) || defined(DEBUG)
#define DBG_CALL(f) f;
#else
#define DBG_CALL(f) nullptr;
#endif

template<typename T0, typename T1>
bool StateCMove(T0 &&dest, T1 &&src) {
	if (dest == src)
		return false;

	dest = src;
	return true;
}

namespace dx12lib {

ID3D12GraphicsCommandList *CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

void CommandList::copyResource(IResource &dest, IResource &src) {
	transitionBarrier(dest, D3D12_RESOURCE_STATE_COPY_DEST);
	transitionBarrier(dest, D3D12_RESOURCE_STATE_COPY_SOURCE);
	_pCommandList->CopyResource(
		dest.getD3DResource().Get(),
		src.getD3DResource().Get()
	);
}

void CommandList::transitionBarrier(const IResource &resource,
	D3D12_RESOURCE_STATES state, UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) {
	_pResourceStateTracker->transitionResource(resource, state, subResource);
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::transitionBarrier(const IResource *pResource,
	D3D12_RESOURCE_STATES state,
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) {
	transitionBarrier(*pResource, state, subResource, flushBarrier);
}

void CommandList::transitionBarrier(std::shared_ptr<IResource> pResource,
	D3D12_RESOURCE_STATES state,
	UINT subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/,
	bool flushBarrier /*= false */) {
	transitionBarrier(pResource.get(), state, subResource, flushBarrier);
}

void CommandList::aliasBarrier(const IResource *pResourceBefore /*= nullptr*/,
	const IResource *pResourceAfter /*= nullptr*/,
	bool flushBarrier /*= false */) {
	_pResourceStateTracker->aliasBarrier(pResourceBefore, pResourceAfter);
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::flushResourceBarriers() {
	_pResourceStateTracker->flushResourceBarriers(shared_from_this());
}

/*************************************************************************************************/

void CommandList::setViewports(const D3D12_VIEWPORT &viewport) {
	_currentGPUState.isSetViewprot = true;
	_pCommandList->RSSetViewports(1, &viewport);
}

void CommandList::setViewprots(const std::vector<D3D12_VIEWPORT> &viewports) {
	assert(!viewports.empty());
	_currentGPUState.isSetViewprot = true;
	_pCommandList->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
}

void CommandList::setScissorRects(const D3D12_RECT &rect) {
	_currentGPUState.isSetScissorRect = true;
	_pCommandList->RSSetScissorRects(1, &rect);
}

void CommandList::setScissorRects(const std::vector<D3D12_RECT> &rects) {
	assert(!rects.empty());
	_currentGPUState.isSetScissorRect = true;
	_pCommandList->RSSetScissorRects(static_cast<UINT>(rects.size()), rects.data());
}

void CommandList::setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) {
	assert(pRenderTarget != nullptr);
	DBG_CALL(_currentGPUState.setRenderTarget(pRenderTarget.get()));
	
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilViews;
	for (std::size_t i = 0; i < AttachmentPoint::DepthStencil; ++i) {
		auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(AttachmentPoint(i));
		if (pRenderTargetBuffer != nullptr)
			renderTargetViews.emplace_back(pRenderTargetBuffer->getRenderTargetView());
	}

	if (auto pDepthStencilBuffer = pRenderTarget->getDepthStencilBuffer())
		depthStencilViews.emplace_back(pDepthStencilBuffer->getDepthStencilView());

	assert((renderTargetViews.size() + depthStencilViews.size()) != 0);
	_pCommandList->OMSetRenderTargets(
		static_cast<UINT>(renderTargetViews.size()),
		renderTargetViews.data(),
		static_cast<UINT>(depthStencilViews.size()),
		depthStencilViews.data()
	);
}

std::shared_ptr<VertexBuffer> 
CommandList::createVertexBuffer(const void *pData, std::size_t sizeInByte, std::size_t stride) {
	return std::make_shared<MakeVertexBuffer>(_pDevice, 
		shared_from_this(), 
		pData, 
		uint32(sizeInByte), 
		uint32(stride)
	);
}

std::shared_ptr<IndexBuffer> 
CommandList::createIndexBuffer(const void *pData, std::size_t sizeInByte, DXGI_FORMAT indexFormat) {
	return std::make_shared<MakeIndexBuffer>(_pDevice, 
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
	return std::make_shared<MakeConstantBuffer>(desc);
}

void CommandList::setVertexBuffer(std::shared_ptr<VertexBuffer> pVertBuffer, UINT slot /*= 0 */) {
	assert(pVertBuffer != nullptr);
	assert(slot < kVertexBufferSlotCount);
	assert(_currentGPUState.pPSO != nullptr);
	if (StateCMove(_currentGPUState.pVertexBuffers[slot], pVertBuffer.get())) {
		_pCommandList->IASetVertexBuffers(
			slot,
			1,
			RVPtr(pVertBuffer->getVertexBufferView())
		);
	}

}

void CommandList::setIndexBuffer(std::shared_ptr<IndexBuffer> pIndexBuffer) {
	assert(pIndexBuffer != nullptr);
	assert(_currentGPUState.pPSO != nullptr);
	if (StateCMove(_currentGPUState.pIndexBuffer, pIndexBuffer.get()))
		_pCommandList->IASetIndexBuffer(RVPtr(pIndexBuffer->getIndexBufferView()));
}

void CommandList::setConstantBufferView(std::shared_ptr<ConstantBuffer> pConstantBuffer, 
	uint32 rootIndex, 
	uint32 offset)
{
	assert(pConstantBuffer != nullptr);
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex, 
		offset, 
		1, 
		pConstantBuffer->getConstantBufferView()
	);
}

void CommandList::setPipelineStateObject(std::shared_ptr<GraphicsPSO> pPipelineStateObject) {
	assert(pPipelineStateObject != nullptr);
	assert(!pPipelineStateObject->isDirty());
	if (StateCMove(_currentGPUState.pPSO, pPipelineStateObject.get())) {
		setGrahicsRootSignature(pPipelineStateObject->getRootSignature());
		_pCommandList->SetPipelineState(pPipelineStateObject->getPipelineStateObject().Get());
	}
}

std::shared_ptr<StructedBuffer> CommandList::createStructedBuffer(const void *pData, std::size_t sizeInByte) {
	return nullptr;
}

std::shared_ptr<UnorderedAccessBuffer> CommandList::createUnorderedAccessBuffer(std::size_t sizeInByte) {
	return nullptr;
}

std::shared_ptr<ReadbackBuffer> CommandList::createReadbackBuffer(std::size_t sizeInByte) {
	return nullptr;
}

void CommandList::setStructedBuffer(std::shared_ptr<StructedBuffer> pStructedBuffer, 
	uint32 rootIndex, 
	uint32 offset /*= 0 */)
{

}

void CommandList::setUnorderedAccessBuffer(std::shared_ptr<UnorderedAccessBuffer> pBuffer, 
	uint32 rootIndex, 
	uint32 offset /*= 0 */) 
{

}

void CommandList::setGrahicsRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	setRootSignature(pRootSignature, &ID3D12GraphicsCommandList::SetGraphicsRootSignature);
}

void CommandList::setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) {
	if (_currentGPUState.primitiveTopology != topology) {
		_currentGPUState.primitiveTopology = topology;
		_pCommandList->IASetPrimitiveTopology(topology);
	}
}

void CommandList::setShaderResourceView(std::shared_ptr<Texture> pTexture, uint32 rootIndex, uint32 offset /*= 0*/) {
	assert(pTexture != nullptr);
	assert(pTexture->checkSRVSupport());
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		pTexture->getShaderResourceView()
	);
}

void CommandList::setStencilRef(UINT stencilRef) {
	if (stencilRef != _currentGPUState.stencilRef) {
		_currentGPUState.stencilRef = stencilRef;
		_pCommandList->OMSetStencilRef(stencilRef);
	}
}

void CommandList::drawInstanced(uint32 vertCount, 
	uint32 instanceCount, 
	uint32 baseVertexLocation,
	uint32 startInstanceLocation)
{
	assert(_currentGPUState.debugCheckDraw());
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawInstanced(vertCount, instanceCount, baseVertexLocation, startInstanceLocation);
}

void CommandList::drawIndexdInstanced(uint32 indexCountPerInstance, 
	uint32 instanceCount, 
	uint32 startIndexLocation, 
	uint32 baseVertexLocation, 
	uint32 startInstanceLocation) 
{
	assert(_currentGPUState.debugCheckDrawIndex());
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawIndexedInstanced(indexCountPerInstance, 
		instanceCount, 
		startIndexLocation, 
		baseVertexLocation,
		startIndexLocation
	);
}

void CommandList::clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float4 color) {
	_pCommandList->ClearRenderTargetView(
		pResource->getRenderTargetView(),
		reinterpret_cast<FLOAT *>(&color),
		0,
		nullptr
	);
}

void CommandList::clearColor(std::shared_ptr<RenderTargetBuffer> pResource, float colors[4]) {
	_pCommandList->ClearRenderTargetView(
		pResource->getRenderTargetView(),
		colors,
		0,
		nullptr
	);
}

void CommandList::clearDepth(std::shared_ptr<DepthStencilBuffer> pResource, float depth) {
	_pCommandList->ClearDepthStencilView(
		pResource->getDepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH,
		depth,
		0,
		0,
		nullptr
	);
}

void CommandList::clearStencil(std::shared_ptr<DepthStencilBuffer> pResource, UINT stencil) {
	_pCommandList->ClearDepthStencilView(
		pResource->getDepthStencilView(),
		D3D12_CLEAR_FLAG_STENCIL,
		0.f,
		stencil,
		0,
		nullptr
	);
}

void CommandList::clearDepthStencil(std::shared_ptr<DepthStencilBuffer> pResource, float depth, UINT stencil) {
	_pCommandList->ClearDepthStencilView(
		pResource->getDepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		depth,
		stencil,
		0,
		nullptr
	);
}

std::shared_ptr<Texture> CommandList::createDDSTextureFromFile(const std::wstring &fileName) {
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromFile12(_pDevice.lock()->getD3DDevice(), 
		_pCommandList.Get(), 
		fileName.c_str(),
		pTexture,
		pUploadHeap
	);
	assert(pTexture != nullptr && pUploadHeap != nullptr);
	return std::make_shared<MakeTexture>(_pDevice, 
		pTexture, 
		pUploadHeap, 
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
}

std::shared_ptr<dx12lib::Texture> CommandList::createDDSTextureFromMemory(const void *pData, 
	std::size_t sizeInByte) 
{
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromMemory12(_pDevice.lock()->getD3DDevice(),
		_pCommandList.Get(),
		reinterpret_cast<const uint8_t *>(pData),
		sizeInByte,
		pTexture,
		pUploadHeap
	);
	assert(pTexture != nullptr && pUploadHeap != nullptr);
	return std::make_shared<MakeTexture>(_pDevice,
		pTexture,
		pUploadHeap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
}

void CommandList::setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, 
	WRL::ComPtr<ID3D12DescriptorHeap> pHeap) 
{
	if (_currentGPUState.pDescriptorHeaps[heapType] != pHeap.Get()) {
		_currentGPUState.pDescriptorHeaps[heapType] = pHeap.Get();
		bindDescriptorHeaps();
	}
}

/**************************************************************************************************/

void CommandList::dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) {
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDispatch(shared_from_this());
	_pCommandList->Dispatch(
		static_cast<UINT>(GroupCountX), 
		static_cast<UINT>(GroupCountY), 
		static_cast<UINT>(GroupCountZ)
	);
}

void CommandList::setComputeRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	setRootSignature(pRootSignature, &ID3D12GraphicsCommandList::SetComputeRootSignature);
}

void CommandList::setPipelineStateObject(std::shared_ptr<ComputePSO> pPipelineStateObject) {
	assert(pPipelineStateObject != nullptr);
	assert(!pPipelineStateObject->isDirty());
	if (StateCMove(_currentGPUState.pPSO, pPipelineStateObject.get())) {
		setComputeRootSignature(pPipelineStateObject->getRootSignature());
		_pCommandList->SetPipelineState(pPipelineStateObject->getPipelineStateObject().Get());
	}
}

/**************************************************************************************************/

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

	_pResourceStateTracker = std::make_unique<MakeResourceStateTracker>();

	for (std::size_t i = 0; i < kDynamicDescriptorHeapCount; ++i) {
		_pDynamicDescriptorHeaps[i] = std::make_unique<MakeDynamicDescriptorHeap>(
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
	std::memset(&_currentGPUState, 0, sizeof(_currentGPUState));
	for (auto &pDynamicDescriptorHeap : _pDynamicDescriptorHeaps)
		pDynamicDescriptorHeap->reset();
}

void CommandList::setRootSignature(std::shared_ptr<RootSignature> pRootSignature, 
	const SetRootSignatureFunc &setFunc) 
{
	if (_currentGPUState.pRootSignature != pRootSignature.get()) {
		_currentGPUState.pRootSignature = pRootSignature.get();
		setFunc(_pCommandList.Get(), pRootSignature->getRootSignature().Get());
		for (auto &pHeap : _pDynamicDescriptorHeaps)
			pHeap->parseRootSignature(pRootSignature);
	}
}

void CommandList::bindDescriptorHeaps() {
	UINT numDescriptors = 0;
	ID3D12DescriptorHeap *pHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { nullptr };
	for (std::size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		if (auto *pHeap = _currentGPUState.pDescriptorHeaps[i]) {
			pHeaps[numDescriptors] = pHeap;
			++numDescriptors;
		}
	}
	if (numDescriptors > 0)
		_pCommandList->SetDescriptorHeaps(numDescriptors, pHeaps);
}

#define CheckState(ret, message)			\
do {										\
	if (!(ret)) {							\
		std::cerr << message << std::endl;	\
		assert(false);						\
		return false;						\
	}										\
} while (false)
bool CommandList::CommandListState::debugCheckDraw() const {
	return pPSO != nullptr && pRootSignature != nullptr && pRenderTarget != nullptr && 
		   isSetViewprot && isSetScissorRect && checkVertexBuffer() && checkTextures() &&
		   primitiveTopology != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	CheckState(pPSO != nullptr, "PipelineStateObject not set");
	CheckState(dynamic_cast<GraphicsPSO *>(pPSO), "PipelineStateObject cast to GraphicsPSO failed!");
	CheckState(pRootSignature != nullptr, "RootSignature not set");
	CheckState(pRenderTarget != nullptr, "RenderTarget not set");
	CheckState(isSetViewprot, "Viewprot not set");
	CheckState(isSetScissorRect, "ScissorRect not set");
	CheckState(checkVertexBuffer(), "No bound vertex buffer");
	CheckState(checkTextures(), "No binding render textures");
	CheckState(primitiveTopology != D3D_PRIMITIVE_TOPOLOGY_UNDEFINED, "unknow primitive topology");
	return true;
}

bool CommandList::CommandListState::debugCheckDrawIndex() const {
	if (!debugCheckDraw())
		return false;

	CheckState(pIndexBuffer != nullptr, "No bound index buffer");
	return true;
}
#undef CheckState

bool CommandList::CommandListState::checkVertexBuffer() const {
	for (auto *pVertexBuffer : pVertexBuffers) {
		if (pVertexBuffers != nullptr)
			return true;
	}
	return false;
}

bool CommandList::CommandListState::checkTextures() const {
	for (auto *pTextures : pVertexBuffers) {
		if (pTextures != nullptr)
			return true;
	}
	return false;
}

void CommandList::CommandListState::setRenderTarget(RenderTarget *pRenderTarget) {
	this->pRenderTarget = pRenderTarget;
	for (std::size_t i = 0; i < AttachmentPoint::NumAttachmentPoints; ++i) {
		pTextures[i] = nullptr;
		if (auto pTexture = pRenderTarget->getTexture(static_cast<AttachmentPoint>(i)))
			pTextures[i] = pTexture.get();
	}
}

}