#pragma once
#include <string>
#include <RenderGraph/RenderGraphStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

namespace rg {

class Pass : public NonCopyable {
public:
	explicit Pass(const std::string &passName);
	virtual void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const = 0;
	virtual void reset() {}
	const std::string &getPassName() const;
private:
	std::string _passName;
};

}
