#include "RenderTarget.h"
#include "Texture.h"

namespace dx12lib {

void swap(RenderTarget &lhs, RenderTarget &rhs) noexcept {
	using std::swap;
	swap(lhs._size, rhs._size);
	swap(lhs._textures, rhs._textures);
}

RenderTarget::RenderTarget(uint32 width, uint32 height) : _size(width, height) {
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

	pTexture->resize(_size.x, _size.y);
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
	for (auto &pTexture : _textures) {
		if (pTexture != nullptr) {
			auto index = formatArray.NumRenderTargets;
			formatArray.RTFormats[index] = pTexture->getResourceDesc().Format;
			formatArray.NumRenderTargets = index + 1;
		}
	}
	return formatArray;
}

DXGI_FORMAT RenderTarget::getDepthStencilFormat() const {
	DXGI_FORMAT ret = DXGI_FORMAT_UNKNOWN;
	auto index = static_cast<std::size_t>(AttachmentPoint::DepthStencil);
	if (_textures[index] != nullptr)
		ret = _textures[index]->getResourceDesc().Format;
	return ret;
}

DXGI_SAMPLE_DESC RenderTarget::getSampleDesc() const {
	DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };
	for (auto &pTexture : _textures) {
		if (pTexture != nullptr) {
			sampleDesc = pTexture->getResourceDesc().SampleDesc;
			break;
		}
	}
	return sampleDesc;
}

}