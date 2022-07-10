#include "SamplerTextureBindable.h"

namespace rg {

SamplerTextureBindable::SamplerTextureBindable(dx12lib::ShaderRegister shaderRegister,
	std::shared_ptr<dx12lib::IShaderResource> pShaderResource,
	size_t mipMap)
: Bindable(BindableType::SamplerTexture)
, _mipMap(mipMap)
, _shaderRegister(shaderRegister)
, _pShaderResource(pShaderResource)
{
}

void SamplerTextureBindable::bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	const auto &srv = _pShaderResource->getSRV(_mipMap);
	pGraphicsCtx->setShaderResourceView(_shaderRegister, srv);
}

}
