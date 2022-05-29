#include "D3D/dx12libHelper/RenderTarget.h"
#include "dx12lib/Texture/RenderTargetTexture.h"
#include "dx12lib/Device/SwapChain.h"
#include "dx12lib/Texture/DepthStencilTexture.h"

namespace d3d {

RenderTarget::RenderTarget(RenderTarget &&other) noexcept
: _pRenderTarget2D(std::move(other._pRenderTarget2D))
, _pDepthStencil2D(std::move(other._pDepthStencil2D))
{
	_width = other._width;
	_height = other._height;
	_isBinding = other._isBinding;
	_finalState = other._finalState;
}

RenderTarget::RenderTarget(std::shared_ptr<dx12lib::RenderTarget2D> pRenderTarget2D,
	                           std::shared_ptr<dx12lib::DepthStencil2D> pDepthStencil2D)
	: _pRenderTarget2D(pRenderTarget2D), _pDepthStencil2D(pDepthStencil2D) {
	assert(pRenderTarget2D->getWidth() == pDepthStencil2D->getWidth());
	assert(pRenderTarget2D->getHeight() == pDepthStencil2D->getHeight());
	_width = pRenderTarget2D->getWidth();
	_height = pRenderTarget2D->getHeight();
}

RenderTarget::RenderTarget(std::shared_ptr<dx12lib::SwapChain> pSwapChain)
	: RenderTarget(pSwapChain->getRenderTarget2D(), pSwapChain->getDepthStencil2D()) {
}

RenderTarget::RenderTarget(dx12lib::GraphicsContextProxy pGraphicsCtx,
	size_t width,
	size_t height,
	DXGI_FORMAT renderTargetFormat,
	DXGI_FORMAT depthStencilFormat)
	: _width(width), _height(height) {
	_pRenderTarget2D = pGraphicsCtx->createRenderTarget2D(width, height, nullptr, renderTargetFormat);
	_pDepthStencil2D = pGraphicsCtx->createDepthStencil2D(width, height, nullptr, depthStencilFormat);
}

std::shared_ptr<dx12lib::RenderTarget2D> RenderTarget::getRenderTarget2D() const {
	return _pRenderTarget2D;
}

std::shared_ptr<dx12lib::DepthStencil2D> RenderTarget::getDepthTarget2D() const {
	return _pDepthStencil2D;
}

void RenderTarget::clear(dx12lib::GraphicsContextProxy pCommonCtx, const Math::float4 &color, float depth, UINT stencil) {
	assert(_isBinding);
	pCommonCtx->clearColor(_pRenderTarget2D, color);
	pCommonCtx->clearDepthStencil(_pDepthStencil2D, depth, stencil);
}

void RenderTarget::bind(dx12lib::GraphicsContextProxy pCommonCtx) {
	_isBinding = true;
	pCommonCtx->setViewport(getViewport());
	pCommonCtx->setScissorRect(getScissorRect());
	pCommonCtx->transitionBarrier(_pRenderTarget2D, D3D12_RESOURCE_STATE_RENDER_TARGET);
	pCommonCtx->setRenderTarget(_pRenderTarget2D->getRTV(), _pDepthStencil2D->getDSV());
}

void RenderTarget::unbind(dx12lib::CommonContextProxy pCommonCtx) {
	_isBinding = false;
	pCommonCtx->transitionBarrier(_pRenderTarget2D, _finalState);
}

D3D12_VIEWPORT RenderTarget::getViewport() const {
	return D3D12_VIEWPORT{
		.TopLeftX = 0.f,
		.TopLeftY = 0.f,
		.Width = static_cast<float>(_width),
		.Height = static_cast<float>(_height),
		.MinDepth = 0.f,
		.MaxDepth = 1.f
	};
}

D3D12_RECT RenderTarget::getScissorRect() const {
	return D3D12_RECT{
		.left = 0,
		.top = 0,
		.right = static_cast<LONG>(_width),
		.bottom = static_cast<LONG>(_height),
	};
}

float2 RenderTarget::getRenderTargetSize() const {
	return float2{ static_cast<float>(_width), static_cast<float>(_height) };
}

float2 RenderTarget::getInvRenderTargetSize() const {
	return float2{ 1.f / static_cast<float>(_width), 1.f / static_cast<float>(_height) };
}

void RenderTarget::setFinalState(D3D12_RESOURCE_STATES finalState) {
	_finalState = finalState;
}

RenderTarget::~RenderTarget() {
	if (_pRenderTarget2D != nullptr) {
		assert(!_isBinding);
	}
}

}


