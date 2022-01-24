#include "RenderTarget.h"
#include "Texture.h"

namespace dx12lib {

void swap(RenderTarget &lhs, RenderTarget &rhs) noexcept {
	using std::swap;
	swap(lhs._size, rhs._size);
	swap(lhs._textures, rhs._textures);
}

void RenderTarget::reset() {
	std::fill(_textures.begin(), _textures.end(), nullptr);
}

void RenderTarget::resize(DX::XMUINT2 size) {
	resize(size.x, size.y);
}

void RenderTarget::resize(uint32 width, uint32 height) {
	_size = { width, height };
	for (auto &pTexture : _textures) {
		if (pTexture != nullptr)
			pTexture->resize(width, height);
	}
}

void RenderTarget::attachTexture(AttachmentPoint point, std::shared_ptr<Texture> pTexture) {
	if (pTexture == nullptr || pTexture->getResource() == nullptr)
		return;
	auto index = static_cast<std::size_t>(point);
	_textures[index] = pTexture;
}

std::shared_ptr<Texture> RenderTarget::getTexture(AttachmentPoint point) const {
	auto index = static_cast<std::size_t>(point);
	return _textures[index];
}

uint32 RenderTarget::getWidth() const noexcept {
	return _size.x;
}

uint32 RenderTarget::getHeight() const noexcept {
	return _size.y;
}

DX::XMUINT2 RenderTarget::getSize() const noexcept {
	return _size;
}

DXGI_FORMAT RenderTarget::getDepthStencilFormat() const {
	DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
	auto index = static_cast<std::size_t>(AttachmentPoint::DepthStencil);
	if (_textures[index] != nullptr)
		ret = _textures[index]->getResourceDesc().Format;
	return ret;
}

}