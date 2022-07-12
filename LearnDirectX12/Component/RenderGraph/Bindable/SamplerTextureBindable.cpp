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

void SamplerTextureBindable::bind(dx12lib::IGraphicsContext &graphicsCtx) const {
	const auto &srv = _pShaderResource->getSRV(_mipMap);
	graphicsCtx.setShaderResourceView(_shaderRegister, srv);
}

}
