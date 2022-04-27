#pragma once
#include "dx12libStd.h"
#include "ContextProxy.hpp"
#include <array>

namespace dx12lib {

class RenderTarget {
protected:
	constexpr static std::size_t kAttachmentPointSize = static_cast<std::size_t>(AttachmentPoint::NumAttachmentPoints);
	RenderTarget(uint32 width, uint32 height);
public:
	RenderTarget(const RenderTarget &) = delete;
	RenderTarget(RenderTarget &&) = default;
	RenderTarget &operator=(const RenderTarget &) = delete;
	RenderTarget &operator=(RenderTarget &&) = default;
	void reset();
	void attachRenderTargetBuffer(AttachmentPoint point, std::shared_ptr<RenderTarget2D> pBuffer);
	void attachDepthStencilBuffer(std::shared_ptr<DepthStencilBuffer> pBuffer);
	std::shared_ptr<RenderTarget2D> getRenderTargetBuffer(AttachmentPoint point) const;
	std::shared_ptr<DepthStencilBuffer> getDepthStencilBuffer() const;
	uint32 getWidth() const noexcept;
	uint32 getHeight() const noexcept;
	DX::XMFLOAT2 getRenderTargetSize() const noexcept;
	DX::XMFLOAT2 getInvRenderTargetSize() const noexcept;
	D3D12_VIEWPORT getViewport(DX::XMFLOAT2 scale = { 1.f, 1.f }, 
		DX::XMFLOAT2 bias = { 0.f, 0.f }, 
		float minDepth = 0.f, 
		float maxDepth = 1.0
	) const;

	D3D12_RT_FORMAT_ARRAY getRenderTargetFormats() const;
	DXGI_FORMAT getDepthStencilFormat() const;
	D3D12_RECT getScissiorRect() const;
	void transitionBarrier(CommonContextProxy pCmdProxy,
		D3D12_RESOURCE_STATES state, 
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);
	friend void swap(RenderTarget &lhs, RenderTarget &rhs) noexcept;
private:
	DX::XMUINT2 _size;
	std::shared_ptr<RenderTarget2D> _pRenderTargetBuffers[NumAttachmentPoints];
	std::shared_ptr<DepthStencilBuffer> _pDepthStencilBuffer;
};

class RenderTargetTransitionBarrier {
public:
	RenderTargetTransitionBarrier(CommonContextProxy pCmdList,
		std::shared_ptr<RenderTarget> pRenderTarget, 
		D3D12_RESOURCE_STATES stateBeforce,
		D3D12_RESOURCE_STATES stateAfter
	);
	~RenderTargetTransitionBarrier();
private:
	CommonContextProxy           _pCmdPorxy;
	std::shared_ptr<RenderTarget> _pRenderTarget;
	D3D12_RESOURCE_STATES         _stateBeforce;
	D3D12_RESOURCE_STATES         _stateAfter;
};

}