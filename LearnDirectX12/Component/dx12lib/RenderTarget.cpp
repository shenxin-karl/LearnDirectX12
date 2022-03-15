#include "RenderTarget.h"
#include "Texture.h"
#include "CommandList.h"
#include "RenderTargetBuffer.h"
#include "DepthStencilBuffer.h"

namespace dx12lib {

RenderTarget::RenderTarget(uint32 width, uint32 height) : _size(width, height) {
}

void RenderTarget::reset() {
	_pDepthStencilBuffer = nullptr;
	for (auto &pBuffer : _pRenderTargetBuffers)
		pBuffer = nullptr;
}

void RenderTarget::attachRenderTargetBuffer(AttachmentPoint point, std::shared_ptr<RenderTargetBuffer> pBuffer) {
	assert(pBuffer != nullptr);
	assert(point < DepthStencil);
	_pRenderTargetBuffers[point] = pBuffer;
}

void RenderTarget::attachDepthStencilBuffer(std::shared_ptr<DepthStencilBuffer> pBuffer) {
	assert(pBuffer != nullptr);
	_pDepthStencilBuffer = pBuffer;
}

std::shared_ptr<RenderTargetBuffer> RenderTarget::getRenderTargetBuffer(AttachmentPoint point) const {
	assert(point >= Color0 && point < DepthStencil);
	return _pRenderTargetBuffers[point];
}

std::shared_ptr<DepthStencilBuffer> RenderTarget::getDepthStencilBuffer() const {
	return _pDepthStencilBuffer;
}

uint32 RenderTarget::getWidth() const noexcept {
	return _size.x;
}

uint32 RenderTarget::getHeight() const noexcept {
	return _size.y;
}

DX::XMFLOAT2 RenderTarget::getRenderTargetSize() const noexcept {
	return DX::XMFLOAT2(float(_size.x), float(_size.y));
}

DX::XMFLOAT2 RenderTarget::getInvRenderTargetSize() const noexcept {
	return DX::XMFLOAT2(1.f / _size.x, 1.f / _size.y);
}

D3D12_VIEWPORT RenderTarget::getViewport(DX::XMFLOAT2 scale /*= { 1.f, 1.f }*/, 
	DX::XMFLOAT2 bias /*= { 0.f, 0.f }*/, 
	float minDepth /*= 0.f*/, 
	float maxDepth /*= 1.0 */) const 
{
	D3D12_VIEWPORT viewport = {
		(_size.x * bias.x),
		(_size.y * bias.y),
		(_size.x * scale.x),
		(_size.y * scale.y),
		minDepth,
		maxDepth,
	};
	return viewport;
}

D3D12_RT_FORMAT_ARRAY RenderTarget::getRenderTargetFormats() const {
	D3D12_RT_FORMAT_ARRAY formatArray = {};
	formatArray.NumRenderTargets = 0;
	for (auto &pBuffer : _pRenderTargetBuffers) {
		if (pBuffer != nullptr) {
			auto index = formatArray.NumRenderTargets;
			formatArray.RTFormats[index] = pBuffer->getD3DResource()->GetDesc().Format;
			formatArray.NumRenderTargets = index + 1;
		}
	}
	return formatArray;
}

DXGI_FORMAT RenderTarget::getDepthStencilFormat() const {
	DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
	if (_pDepthStencilBuffer != nullptr) 
		ret = _pDepthStencilBuffer->getD3DResource()->GetDesc().Format;
	return ret;
}


D3D12_RECT RenderTarget::getScissiorRect() const {
	return D3D12_RECT(0, 0, _size.x, _size.y);
}

void RenderTarget::transitionBarrier(CommandListProxy pCmdList, 
	D3D12_RESOURCE_STATES state, 
	UINT subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES */) 
{
	for (std::size_t i = 0; i < AttachmentPoint::DepthStencil; ++i) {
		auto pRenderTargetBuffer = getRenderTargetBuffer(static_cast<AttachmentPoint>(i));
		if (pRenderTargetBuffer != nullptr) {
			pCmdList->transitionBarrier(
				std::static_pointer_cast<IResource>(pRenderTargetBuffer),
				state,
				subresource
			);
		}
	}
}

void swap(RenderTarget &lhs, RenderTarget &rhs) noexcept {
	using std::swap;
	swap(lhs._size, rhs._size);
	swap(lhs._pRenderTargetBuffers, rhs._pRenderTargetBuffers);
	swap(lhs._pDepthStencilBuffer, rhs._pDepthStencilBuffer);
}

/*************************************************************************************/

RenderTargetTransitionBarrier::RenderTargetTransitionBarrier(CommandListProxy pCmdList, 
	std::shared_ptr<RenderTarget> pRenderTarget, 
	D3D12_RESOURCE_STATES stateBeforce, 
	D3D12_RESOURCE_STATES stateAfter)
: _pCmdList(pCmdList), _pRenderTarget(pRenderTarget), _stateBeforce(stateBeforce), _stateAfter(stateAfter)
{
	_pRenderTarget->transitionBarrier(_pCmdList, _stateBeforce);
}

RenderTargetTransitionBarrier::~RenderTargetTransitionBarrier() {
	_pRenderTarget->transitionBarrier(_pCmdList, _stateAfter);
}

}