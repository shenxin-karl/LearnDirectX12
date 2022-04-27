#include "CommandList.h"
#include "Device.h"
#include "FrameResourceQueue.h"
#include "RenderTarget.h"
#include "ResourceStateTracker.h"
#include "DynamicDescriptorHeap.h"
#include "PipelineStateObject.h"
#include "ConstantBuffer.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "CommandQueue.h"
#include "RootSignature.h"
#include "MakeObejctTool.hpp"
#include "DDSTextureLoader.h"
#include "RenderTargetBuffer.h"
#include "DepthStencilBuffer.h"
#include "ShaderResourceBuffer.h"
#include "ReadBackBuffer.h"
#include "StructuredBuffer.h"
#include "UnorderedAccessBuffer.h"
#include "FRStructuredBuffer.hpp"
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

/// private function
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

	_pResourceStateTracker = std::make_unique<dx12libTool::MakeResourceStateTracker>();
	for (std::size_t i = 0; i < kDynamicDescriptorHeapCount; ++i) {
		_pDynamicDescriptorHeaps[i] = std::make_unique<dx12libTool::MakeDynamicDescriptorHeap>(
			_pDevice,
			static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i),
			static_cast<uint32>(kDynamicDescriptorPerHeap)
		);
	}
}

CommandList::~CommandList() {
}

/// ******************************************** Context api ***************************************************
ID3D12GraphicsCommandList *CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

std::weak_ptr<dx12lib::Device> CommandList::getDevice() const {
	return _pDevice;
}

/// ******************************************** CommonContext api ********************************************
std::shared_ptr<ConstantBuffer>
CommandList::createConstantBuffer(std::size_t sizeInByte, const void *pData) {
	return std::make_shared<dx12libTool::MakeConstantBuffer>(_pDevice, pData, sizeInByte);
}

std::shared_ptr<FRConstantBuffer<>> CommandList::createFRRawConstantBuffer(size_t sizeInByte, const void *pData) {
	assert(pData != nullptr && sizeInByte > 0);
	return std::make_shared<dx12libTool::MakeFRRawConstantBuffer>(
		_pDevice,
		sizeInByte,
		pData
		);
}

std::shared_ptr<StructuredBuffer> CommandList::createStructuredBuffer(const void *pData, size_t numElements, size_t stride) {
	assert(pData != nullptr && numElements > 0 && stride > 0);
	return std::make_shared<dx12libTool::MakeStructuredBuffer>(
		_pDevice,
		pData,
		numElements,
		stride
	);
}

std::shared_ptr<FRStructuredBuffer<RawData>> CommandList::createFRRawStructuredBuffer(const void *pData, size_t numElements, size_t stride) {
	assert(pData != nullptr);
	assert(numElements > 0);
	assert(stride > 0);
	return std::make_shared<dx12libTool::MakeFRRawStructuredBuffer>(
		_pDevice,
		pData,
		numElements,
		stride
	);
}

std::shared_ptr<Texture2D> CommandList::createDDSTexture2DFromFile(const std::wstring &fileName) {
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromFile12(_pDevice.lock()->getD3DDevice(),
		_pCommandList.Get(),
		fileName.c_str(),
		pTexture,
		pUploadHeap
	);
	assert(pTexture != nullptr && pUploadHeap != nullptr);
	assert(pTexture->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
	return std::make_shared<dx12libTool::MakeTexture2D>(_pDevice,
		pTexture,
		pUploadHeap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
}

std::shared_ptr<Texture2D> CommandList::createDDSTexture2DFromMemory(const void *pData,
	std::size_t sizeInByte) {
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromMemory12(_pDevice.lock()->getD3DDevice(),
		_pCommandList.Get(),
		static_cast<const uint8_t *>(pData),
		sizeInByte,
		pTexture,
		pUploadHeap
	);
	assert(pTexture != nullptr && pUploadHeap != nullptr);
	assert(pTexture->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
	return std::make_shared<dx12libTool::MakeTexture2D>(_pDevice,
		pTexture,
		pUploadHeap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
}

std::shared_ptr<ShaderResourceBuffer> CommandList::createDDSTextureCubeFromFile(const std::wstring &fileName) {
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromFile12(_pDevice.lock()->getD3DDevice(),
		_pCommandList.Get(),
		fileName.c_str(),
		pTexture,
		pUploadHeap
	);

	assert(pTexture != nullptr && pUploadHeap != nullptr);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = pTexture->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = pTexture->GetDesc().Format;
	return std::make_shared<dx12libTool::MakeShaderResourceBuffer>(_pDevice,
		pTexture,
		pUploadHeap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&srvDesc
	);
}

std::shared_ptr<ShaderResourceBuffer> CommandList::createDDSTextureCubeFromMemory(const void *pData, 
	size_t sizeInByte)
{
	WRL::ComPtr<ID3D12Resource> pTexture;
	WRL::ComPtr<ID3D12Resource> pUploadHeap;
	DirectX::CreateDDSTextureFromMemory12(_pDevice.lock()->getD3DDevice(),
		_pCommandList.Get(),
		static_cast<const uint8_t *>(pData),
		sizeInByte,
		pTexture,
		pUploadHeap
	);

	assert(pTexture != nullptr && pUploadHeap != nullptr);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = pTexture->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = pTexture->GetDesc().Format;
	return std::make_shared<dx12libTool::MakeShaderResourceBuffer>(_pDevice,
		pTexture,
		pUploadHeap,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&srvDesc
	);
}

void CommandList::setShaderResourceBufferImpl(std::shared_ptr<IShaderResourceBuffer> pTexture, size_t rootIndex, size_t offset) {
	assert(pTexture != nullptr);
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		pTexture->getShaderResourceView()
	);
}

void CommandList::setDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	WRL::ComPtr<ID3D12DescriptorHeap> pHeap) {
	if (_currentGPUState.pDescriptorHeaps[heapType] != pHeap.Get()) {
		_currentGPUState.pDescriptorHeaps[heapType] = pHeap.Get();
		bindDescriptorHeaps();
	}
}

void CommandList::setConstantBufferView(const ConstantBufferView &cbv, size_t rootIndex, size_t offset) {
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		cbv
	);
}

void CommandList::setShaderResourceView(const ShaderResourceView &srv, size_t rootIndex, size_t offset) {
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		srv
	);
}

void CommandList::setStructuredBuffer(std::shared_ptr<StructuredBuffer> pStructuredBuffer, size_t rootIndex, size_t offset) {
	setStructuredBufferImpl(
		std::static_pointer_cast<IStructuredBuffer>(pStructuredBuffer),
		rootIndex,
		offset
	);
}

void CommandList::setStructuredBufferImpl(std::shared_ptr<IStructuredBuffer> pStructuredBuffer, size_t rootIndex, size_t offset) {
	assert(pStructuredBuffer != nullptr);
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		pStructuredBuffer->getShaderResourceView()
	);
}

void CommandList::setConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer, size_t rootIndex, size_t offset) {
	this->setConstantBufferImpl(
		std::static_pointer_cast<IConstantBuffer>(pConstantBuffer),
		rootIndex,
		offset
	);
}

void CommandList::setConstantBufferImpl(std::shared_ptr<IConstantBuffer> pConstantBuffer,
                                        size_t rootIndex,
                                        size_t offset)
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

void CommandList::copyResourceImpl(std::shared_ptr<IResource> pDest, std::shared_ptr<IResource> pSrc) {
	assert(pDest->getWidth() == pSrc->getWidth());
	assert(pDest->getHeight() == pSrc->getHeight());
	assert(pDest->getFormat() == pSrc->getFormat());
	assert(!pDest->isMapped());
	assert(!pSrc->isMapped());

	transitionBarrier(pDest, D3D12_RESOURCE_STATE_COPY_DEST);
	transitionBarrier(pSrc, D3D12_RESOURCE_STATE_COPY_SOURCE);
	flushResourceBarriers();
	_pCommandList->CopyResource(
		pDest->getD3DResource().Get(),
		pSrc->getD3DResource().Get()
	);
}

void CommandList::transitionBarrierImpl(std::shared_ptr<IResource> pBuffer, 
	D3D12_RESOURCE_STATES state, 
	UINT subResource,
	bool flushBarrier) 
{
	_pResourceStateTracker->transitionResource(pBuffer->getD3DResource().Get(), state, subResource);
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::aliasBarrierImpl(std::shared_ptr<IResource> pBeforce, 
	std::shared_ptr<IResource> pAfter, 
	bool flushBarrier) 
{
	_pResourceStateTracker->aliasBarrier(pBeforce.get(), pAfter.get());
	if (flushBarrier)
		flushResourceBarriers();
}

void CommandList::flushResourceBarriers() {
	_pResourceStateTracker->flushResourceBarriers(shared_from_this());
}

/// ******************************************** GraphicsContext api ********************************************
std::shared_ptr<VertexBuffer>
CommandList::createVertexBuffer(const void *pData, std::size_t numElements, std::size_t stride) {
	return std::make_shared<dx12libTool::MakeVertexBuffer>(_pDevice,
		shared_from_this(),
		pData,
		numElements,
		stride
	);
}

std::shared_ptr<IndexBuffer>
CommandList::createIndexBuffer(const void *pData, std::size_t numElements, DXGI_FORMAT indexFormat) {
	return std::make_shared<dx12libTool::MakeIndexBuffer>(_pDevice,
		shared_from_this(),
		pData,
		numElements,
		indexFormat
	);
}

void CommandList::setViewport(const D3D12_VIEWPORT &viewport) {
	_currentGPUState.isSetViewport = true;
	_pCommandList->RSSetViewports(1, &viewport);
}

void CommandList::setScissorRect(const D3D12_RECT &rect) {
	_currentGPUState.isSetScissorRect = true;
	_pCommandList->RSSetScissorRects(1, &rect);
}

void CommandList::setRenderTarget(std::shared_ptr<RenderTarget> pRenderTarget) {
	assert(pRenderTarget != nullptr);
	DBG_CALL(_currentGPUState.setRenderTarget(pRenderTarget.get()));
	
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> depthStencilViews;
	for (std::size_t i = 0; i < AttachmentPoint::NumAttachmentPoints; ++i) {
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


void CommandList::setGraphicsPSO(std::shared_ptr<GraphicsPSO> pPipelineStateObject) {
	assert(pPipelineStateObject != nullptr);
	assert(!pPipelineStateObject->isDirty());
	if (StateCMove(_currentGPUState.pPSO, pPipelineStateObject.get())) {
		setGraphicsRootSignature(pPipelineStateObject->getRootSignature());
		_pCommandList->SetPipelineState(pPipelineStateObject->getPipelineStateObject().Get());
	}
}

void CommandList::setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY topology) {
	if (_currentGPUState.primitiveTopology != topology) {
		_currentGPUState.primitiveTopology = topology;
		_pCommandList->IASetPrimitiveTopology(topology);
	}
}

void CommandList::setStencilRef(UINT stencilRef) {
	if (stencilRef != _currentGPUState.stencilRef) {
		_currentGPUState.stencilRef = stencilRef;
		_pCommandList->OMSetStencilRef(stencilRef);
	}
}

void CommandList::setGraphics32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset) {
	assert(_currentGPUState.debugCheckSet32BitConstants(rootIndex, numConstants + destOffset));
	_pCommandList->SetGraphicsRoot32BitConstants(
		static_cast<UINT>(rootIndex),
		static_cast<UINT>(numConstants), 
		pData,
		static_cast<UINT>(destOffset)
	);
}


void CommandList::drawInstanced(size_t vertCount,
	size_t instanceCount,
	size_t baseVertexLocation,
	size_t startInstanceLocation)
{
	assert(_currentGPUState.debugCheckDraw());
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawInstanced(
		static_cast<UINT>(vertCount), 
		static_cast<UINT>(instanceCount),
		static_cast<UINT>(baseVertexLocation),
		static_cast<UINT>(startInstanceLocation)
	);
}

void CommandList::drawIndexedInstanced(size_t indexCountPerInstance,
	size_t instanceCount,
	size_t startIndexLocation,
	size_t baseVertexLocation,
	size_t startInstanceLocation)
{
	assert(_currentGPUState.debugCheckDrawIndex());
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDraw(shared_from_this());
	_pCommandList->DrawIndexedInstanced(
		static_cast<UINT>(indexCountPerInstance), 
		static_cast<UINT>(instanceCount), 
		static_cast<UINT>(startIndexLocation),
		static_cast<UINT>(baseVertexLocation),
		static_cast<UINT>(startIndexLocation)
	);
}

void CommandList::clearColor(std::shared_ptr<RenderTarget2D> pResource, float4 color) {
	_pCommandList->ClearRenderTargetView(
		pResource->getRenderTargetView(),
		reinterpret_cast<FLOAT *>(&color),
		0,
		nullptr
	);
}

void CommandList::clearColor(std::shared_ptr<RenderTarget2D> pResource, float colors[4]) {
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


/// ******************************************** ComputeContext api ********************************************

std::shared_ptr<UnorderedAccessBuffer> CommandList::createUnorderedAccessBuffer(std::size_t width,
	std::size_t height,
	DXGI_FORMAT format) {
	assert(format != DXGI_FORMAT::DXGI_FORMAT_UNKNOWN);
	assert(width > 0);
	assert(height > 0);
	return std::make_shared<dx12libTool::MakeUnorderedAccessBuffer>(
		_pDevice,
		width,
		height,
		format
	);
}

std::shared_ptr<ReadBackBuffer> CommandList::createReadBackBuffer(std::size_t sizeInByte) {
	assert(sizeInByte > 0);
	return std::make_shared<dx12libTool::MakeReadBackBuffer>(
		_pDevice,
		sizeInByte
	);
}

void CommandList::setComputePSO(std::shared_ptr<ComputePSO> pPipelineStateObject) {
	assert(pPipelineStateObject != nullptr);
	assert(!pPipelineStateObject->isDirty());
	if (StateCMove(_currentGPUState.pPSO, pPipelineStateObject.get())) {
		setComputeRootSignature(pPipelineStateObject->getRootSignature());
		_pCommandList->SetPipelineState(pPipelineStateObject->getPipelineStateObject().Get());
	}
}

void CommandList::setUnorderedAccessBuffer(std::shared_ptr<UnorderedAccessBuffer> pBuffer,
	size_t rootIndex,
	size_t offset)
{
	assert(pBuffer != nullptr);
	assert(_currentGPUState.pRootSignature != nullptr);
	_pDynamicDescriptorHeaps[0]->stageDescriptors(
		rootIndex,
		offset,
		1,
		pBuffer->getUnorderedAccessView()
	);
}

void CommandList::setCompute32BitConstants(size_t rootIndex, size_t numConstants, const void *pData, size_t destOffset) {
	assert(_currentGPUState.debugCheckSet32BitConstants(rootIndex, numConstants + destOffset));
	_pCommandList->SetComputeRoot32BitConstants(
		static_cast<UINT>(rootIndex), 
		static_cast<UINT>(numConstants),
		pData, 
		static_cast<UINT>(destOffset));
}

void CommandList::dispatch(size_t GroupCountX, size_t GroupCountY, size_t GroupCountZ) {
	assert(GroupCountX >= 1);
	assert(GroupCountY >= 1);
	assert(GroupCountZ >= 1);
	flushResourceBarriers();
	for (auto &pDynamicHeap : _pDynamicDescriptorHeaps)
		pDynamicHeap->commitStagedDescriptorForDispatch(shared_from_this());
	_pCommandList->Dispatch(
		static_cast<UINT>(GroupCountX), 
		static_cast<UINT>(GroupCountY), 
		static_cast<UINT>(GroupCountZ)
	);
}

void CommandList::readBack(std::shared_ptr<ReadBackBuffer> pReadBackBuffer) {
	_pReadBackBuffers.push_back(pReadBackBuffer);
}

void CommandList::setGraphicsRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	setRootSignature(pRootSignature, &ID3D12GraphicsCommandList::SetGraphicsRootSignature);
}

void CommandList::setComputeRootSignature(std::shared_ptr<RootSignature> pRootSignature) {
	setRootSignature(pRootSignature, &ID3D12GraphicsCommandList::SetComputeRootSignature);
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
	std::memset(&_currentGPUState, 0, sizeof(_currentGPUState));
	for (auto &pDynamicDescriptorHeap : _pDynamicDescriptorHeaps)
		pDynamicDescriptorHeap->reset();

	for (auto &pReadBackBuffer : _pReadBackBuffers)
		pReadBackBuffer->setCompleted(true);
	_pReadBackBuffers.clear();
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
	CheckState(pPSO != nullptr, "PipelineStateObject not set");
	CheckState(dynamic_cast<GraphicsPSO *>(pPSO), "PipelineStateObject cast to GraphicsPSO failed!");
	CheckState(pRootSignature != nullptr, "RootSignature not set");
	CheckState(pRenderTarget != nullptr, "RenderTarget not set");
	CheckState(isSetViewport, "Viewprot not set");
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

bool CommandList::CommandListState::debugCheckSet32BitConstants(size_t rootIndex, size_t numConstants) const {
	if (pRootSignature == nullptr)
		return false;
	const auto &desc = pRootSignature->getRootSignatureDesc();
	if (rootIndex > desc.NumParameters)
		return false;
	if (desc.pParameters[rootIndex].ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
		return false;

	return numConstants <= desc.pParameters[rootIndex].Constants.Num32BitValues;
}

void CommandList::CommandListState::setRenderTarget(RenderTarget *pRenderTarget) {
	this->pRenderTarget = pRenderTarget;
	for (std::size_t i = 0; i < AttachmentPoint::NumAttachmentPoints; ++i) {
		pRTBuffers[i] = nullptr;
		if (auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(static_cast<AttachmentPoint>(i)))
			pRTBuffers[i] = pRenderTargetBuffer.get();
	}
	pDepthStencil = pRenderTarget->getDepthStencilBuffer().get();
}

}