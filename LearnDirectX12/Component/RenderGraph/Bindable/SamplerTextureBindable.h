#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.h>
#include <dx12lib/Texture/SamplerTexture.h>
#include <dx12lib/Pipeline/ShaderRegister.hpp>

namespace rg {

class SamplerTextureBindable : public Bindable {
public:
	SamplerTextureBindable(dx12lib::ShaderRegister shaderRegister, std::shared_ptr<dx12lib::IShaderResource> pShaderResource, size_t mipMap);
	void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const override;
private:
	size_t _mipMap = 0;
	dx12lib::ShaderRegister _shaderRegister;
	std::shared_ptr<dx12lib::IShaderResource> _pShaderResource;
};

}
