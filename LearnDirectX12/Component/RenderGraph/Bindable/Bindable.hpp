#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include <source_location>

namespace rg {

enum class BindableType {
	Unknown = 0,
	ConstantBuffer,
	SamplerTexture,
	ViewportScissor,
	PipelineStateObject,
};

#define DECLARE_SOURCE_LOCATION_ARG const std::source_location &sourceLocation = std::source_location::current()

class Bindable : public	NonCopyable {
public:
	Bindable(BindableType bindableType, const std::source_location &sr) : _bindableType(bindableType), _sourceLocation(sr) {}
	virtual void bind(dx12lib::IGraphicsContext &graphicsCtx) const = 0;
	~Bindable() override = default;
	BindableType getBindableType() const {
		return _bindableType;
	}
private:
	BindableType _bindableType;
	std::source_location _sourceLocation;
};

}
