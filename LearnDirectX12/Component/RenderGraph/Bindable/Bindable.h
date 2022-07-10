#pragma once
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

interface Bindable : public	NonCopyable {
	virtual void bind(dx12lib::GraphicsContextProxy pGraphicsCtx);
	~Bindable() override = default;
};

}
