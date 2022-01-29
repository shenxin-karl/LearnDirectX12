#include "CommandList.h"
#include "Device.h"
#include "FrameResourceQueue.h"
#include "RenderTarget.h"
#include "Texture.h"

namespace dx12lib {

ID3D12GraphicsCommandList * CommandList::getD3DCommandList() const noexcept {
	return _pCommandList.Get();
}

HRESULT CommandList::close() {
	return _pCommandList->Close();
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
		depthStencilViews.push_back(pDepthStencilTexture->getDepthStencilView());
	}

	_pCommandList->OMSetRenderTargets(
		static_cast<UINT>(renderTargetViews.size()),
		renderTargetViews.data(),
		static_cast<UINT>(depthStencilViews.size()),
		depthStencilViews.data()
	);
}

CommandList::CommandList(std::weak_ptr<FrameResourceItem> pFrameResourceItem) {
	auto pSharedFrameResourceItem = pFrameResourceItem.lock();
	_cmdListType = pSharedFrameResourceItem->getCommandListType();
	_pDevice = pSharedFrameResourceItem->getDevice();

	auto pDevice = pSharedFrameResourceItem->getDevice();
	auto pd3d12Device = pDevice.lock()->getD3DDevice();
	ThrowIfFailed(pd3d12Device->CreateCommandList(
		0,
		pSharedFrameResourceItem->getCommandListType(),
		pSharedFrameResourceItem->getCommandListAllocator().Get(),
		nullptr,
		IID_PPV_ARGS(&_pCommandList)
	));
}

}