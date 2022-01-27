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

	auto rect = pRenderTarget->getScissiorRect();
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
	for (std::size_t i = 0; i < static_cast<std::size_t>(AttachmentPoint::DepthStencil); ++i) {
		auto pTexture = pRenderTarget->getTexture(static_cast<AttachmentPoint>(i));
		if (pTexture != nullptr && pTexture->checkRTVSupport()) {
			auto handle = pTexture->getRenderTargetView();
			renderTargetViews.push_back(handle);

			// clear render target 
			if (pRenderTarget->isRTVClearValueDirty() && pTexture->getClearValue() != nullptr) {
				_pCommandList->ClearRenderTargetView(
					pTexture->getRenderTargetView(),
					pTexture->getClearValue()->Color,
					1,
					&rect
				);
			}
		}
	}
	
	auto pDepthStencilTexture = pRenderTarget->getTexture(AttachmentPoint::DepthStencil);
	if (pDepthStencilTexture != nullptr && pDepthStencilTexture->checkDSVSupport()) {
		// todo
	}
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