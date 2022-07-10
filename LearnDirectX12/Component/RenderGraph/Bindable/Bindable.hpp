#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

enum class BindableType {
	Unknown = 0,
	ConstantBuffer,
	SamplerTexture,
	ViewportScissor,
	PipelineStateObject,
};

class Bindable : public	NonCopyable {
public:
	virtual void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const = 0;
	~Bindable() override = default;
	BindableType getBindableType() const {
		return _bindableType;
	}
private:
	BindableType _bindableType;
};

}
