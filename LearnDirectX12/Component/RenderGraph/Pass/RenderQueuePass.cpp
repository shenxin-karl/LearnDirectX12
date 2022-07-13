#include "RenderQueuePass.h"
#include "RenderGraph/Bindable/Bindable.hpp"
#include "RenderGraph/Bindable/GraphicsPSOBindable.h"

namespace rg {

SubPass::SubPass(RenderQueuePass *pRenderQueuePass, std::shared_ptr<GraphicsPSOBindable> pGraphicsBindable)
: _pRenderQueuePass(pRenderQueuePass), _pGraphicsPSOBindable(std::move(pGraphicsBindable))
{
}

const std::string & SubPass::getSubPassName() const {
	return _pGraphicsPSOBindable->getPSOName();
}

void SubPass::addBindable(std::shared_ptr<Bindable> pBindable) {
	_bindables.push_back(std::move(pBindable));
}

std::shared_ptr<Bindable> SubPass::getBindableByType(BindableType bindableType) const {
	for (auto &pBindable : _bindables) {
		if (pBindable->getBindableType() == bindableType)
			return pBindable;
	}
	return nullptr;
}

void SubPass::accept(const Job &job) {
	_jobs.push_back(job);
}

void SubPass::execute(dx12lib::IGraphicsContext &graphicsCtx) const {
	_pGraphicsPSOBindable->bind(graphicsCtx);
	for (auto &pBindable : _bindables)
		pBindable->bind(graphicsCtx);

	for (auto &job : _jobs)
		job.execute(graphicsCtx);
}

void SubPass::reset() {
	_jobs.clear();
}

std::shared_ptr<SubPass> RenderQueuePass::getSubPassByName(const std::string &subPassName) const {
	for (auto &pSubPass : _subPasses) {
		if (pSubPass->getSubPassName() == subPassName)
			return pSubPass;
	}
	return nullptr;
}

std::shared_ptr<SubPass> RenderQueuePass::getOrCreateSubPass(std::shared_ptr<GraphicsPSOBindable> pGraphicsPSOBindable) {
	auto subPassName = pGraphicsPSOBindable->getPSOName();
	for (auto &pSubPass : _subPasses) {
		if (pSubPass->getSubPassName() == subPassName)
			return pSubPass;
	}
	_subPasses.push_back(std::make_shared<SubPass>(this, std::move(pGraphicsPSOBindable)));
	return _subPasses.back();
}

void RenderQueuePass::execute(dx12lib::GraphicsContextProxy pGraphicsCtx) const {
	bindAll(*pGraphicsCtx);
	bindRenderTarget(*pGraphicsCtx);
	for (auto &pSubPass : _subPasses)
		pSubPass->execute(*pGraphicsCtx);
}

void RenderQueuePass::reset() {
	BindingPass::reset();
	for (auto &pSubPass : _subPasses)
		pSubPass->reset();
}

}
