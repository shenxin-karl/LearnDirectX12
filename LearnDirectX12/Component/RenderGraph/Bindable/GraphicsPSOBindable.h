#pragma once
#include <RenderGraph/Bindable/Bindable.h>
#include <string>

namespace rg {

class Pass;
class GraphicsPSOBindable : public Bindable {
public:
	GraphicsPSOBindable(std::shared_ptr<dx12lib::GraphicsPSO> pso);
	void bind(dx12lib::GraphicsContextProxy pGraphicsCtx) const override;
	const std::string &getPSOName() const;
private:
	std::shared_ptr<dx12lib::GraphicsPSO> _pso;
};

}
