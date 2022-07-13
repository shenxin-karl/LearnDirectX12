#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <RenderGraph/Bindable/Bindable.hpp>
#include <dx12lib/Resource/IResource.h>
#include <dx12lib/Pipeline/ShaderRegister.hpp>

namespace rg {

class ConstantBufferBindable : public Bindable {
public:
	static std::shared_ptr<ConstantBufferBindable> make(const dx12lib::ShaderRegister &sr,
		std::shared_ptr<dx12lib::IConstantBuffer> pConstantBuffer,
		DECLARE_SOURCE_LOCATION_ARG
	);
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const override;
	ConstantBufferBindable(const std::source_location &sr) : Bindable(BindableType::ConstantBuffer, sr) {}
private:
	dx12lib::ShaderRegister _shaderRegister;
	std::shared_ptr<dx12lib::IConstantBuffer> _pConstantBuffer;
};

}
