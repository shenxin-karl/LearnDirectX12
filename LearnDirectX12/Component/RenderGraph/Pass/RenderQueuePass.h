#pragma once
#include <RenderGraph/Pass/BindingPass.h>
#include <RenderGraph/Pass/Job.h>

namespace rg {

class SubPass {
public:
	SubPass(RenderQueuePass *pRenderQueuePass, std::shared_ptr<GraphicsPSOBindable> pGraphicsBindable);
	const std::string &getSubPassName() const;
	void addBindable(std::shared_ptr<Bindable> pBindable);
	std::shared_ptr<Bindable> getBindableByType(BindableType bindableType) const;
	void accept(const Job &job);
	void execute(dx12lib::IGraphicsContext &graphicsCtx) const;
	void reset();
private:
	std::vector<Job> _jobs;
	RenderQueuePass *_pRenderQueuePass;
	std::shared_ptr<GraphicsPSOBindable> _pGraphicsPSOBindable;
	std::vector<std::shared_ptr<Bindable>> _bindables;
};

class RenderQueuePass : public BindingPass {
public:
	using BindingPass::BindingPass;
	std::shared_ptr<SubPass> getSubPassByName(const std::string &subPassName) const;
	std::shared_ptr<SubPass> getOrCreateSubPass(std::shared_ptr<GraphicsPSOBindable> pGraphicsPSOBindable);
	void execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const override;
	void reset() override;
private:
	std::vector<std::shared_ptr<SubPass>> _subPasses;
};

}
