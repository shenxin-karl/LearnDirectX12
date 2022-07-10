#pragma once
#include <RenderGraph/Pass/BindingPass.h>
#include <RenderGraph/Pass/Job.h>

namespace rg {

class RenderQueuePass : public BindingPass {
public:
	using BindingPass::BindingPass;
	void accept(Job job);
	void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const override;
	void reset() override;
private:
	std::vector<Job> _jobs;
};

}
