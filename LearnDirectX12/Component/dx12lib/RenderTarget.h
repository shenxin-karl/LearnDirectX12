#pragma once
#include "dx12libStd.h"
#include <array>

namespace dx12lib {

class Texture;

enum class AttachmentPoint {
	Color0,
	Color1,
	Color2,
	Color3,
	Color4,
	Color5,
	Color6,
	Color7,
	DepthStencil,
	NumAttachmentPoints,
};

class RenderTarget {
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
	DX::XMUINT2 getSize() const noexcept;

	D3D12_VIEWPORT getViewport(DX::XMFLOAT2 scale = { 1.f, 1.f }, 
		DX::XMFLOAT2 bias = { 0.f, 0.f }, 
		float minDepth = 0.f, 
		float maxDepth = 1.0
	) const;

	D3D12_RT_FORMAT_ARRAY getRenderTargetFormats() const;
	DXGI_FORMAT getDepthStencilFormat() const;
	DXGI_SAMPLE_DESC getSampleDesc() const;
	D3D12_RECT getScissiorRect() const;
private:
	constexpr static std::size_t kAttachmentPointSize = static_cast<std::size_t>(AttachmentPoint::NumAttachmentPoints);
	std::array<std::shared_ptr<Texture>, kAttachmentPointSize>  _textures;
	DX::XMUINT2 _size;
};

}