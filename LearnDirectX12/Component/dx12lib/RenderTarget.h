#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"
#include <array>

namespace dx12lib {

class RenderTarget {
	constexpr static std::size_t kAttachmentPointSize = static_cast<std::size_t>(AttachmentPoint::NumAttachmentPoints);
public:
	RenderTarget(uint32 width, uint32 height);
	RenderTarget(const RenderTarget &) = delete;
	RenderTarget(RenderTarget &&) = default;
	RenderTarget &operator=(const RenderTarget &) = delete;
	RenderTarget &operator=(RenderTarget &&) = default;
	friend void swap(RenderTarget &lhs, RenderTarget &rhs) noexcept;
	void reset();
	void resize(DX::XMUINT2 size);
	void resize(uint32 width, uint32 height);
	void attachTexture(AttachmentPoint point, std::shared_ptr<Texture> pTexture);
	std::shared_ptr<Texture> getTexture(AttachmentPoint point) const;
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
	DXGI_SAMPLE_DESC getSampleDesc() const;
	D3D12_RECT getScissiorRect() const;
	void transitionBarrier(CommandListProxy pCmdList, 
		D3D12_RESOURCE_STATES state, 
		UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
	);
private:
	std::array<std::shared_ptr<Texture>, kAttachmentPointSize>  _textures;
	DX::XMUINT2 _size;
};

class RenderTargetTransitionBarrier {
public:
	RenderTargetTransitionBarrier(CommandListProxy pCmdList, 
		std::shared_ptr<RenderTarget> pRenderTarget, 
		D3D12_RESOURCE_STATES stateBeforce,
		D3D12_RESOURCE_STATES stateAfter
	);
	~RenderTargetTransitionBarrier();
private:
	CommandListProxy              _pCmdList;
	std::shared_ptr<RenderTarget> _pRenderTarget;
	D3D12_RESOURCE_STATES         _stateBeforce;
	D3D12_RESOURCE_STATES         _stateAfter;
};

}