#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.hpp>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Pipeline/ShaderRegister.hpp>

namespace rg {

class ConstantBufferBindable : public Bindable {
public:
	ConstantBufferBindable(const dx12lib::ShaderRegister &sr, std::shared_ptr<dx12lib::IConstantBuffer> pConstantBuffer);
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const override;
private:
	dx12lib::ShaderRegister _shaderRegister;
	std::shared_ptr<dx12lib::IConstantBuffer> _pConstantBuffer;
};

}
