#pragma once
#include <RenderGraph/Bindable/Bindable.hpp>
#include <string>

#include "dx12lib/Context/CommonContext.h"

namespace rg {

class Pass;
class GraphicsPSOBindable : public Bindable {
public:
	GraphicsPSOBindable(std::shared_ptr<dx12lib::GraphicsPSO> pso);
	void bind(dx12lib::IGraphicsContext &graphicsCtx) const override;
	const std::string &getPSOName() const;
private:
	std::shared_ptr<dx12lib::GraphicsPSO> _pso;
};

}
