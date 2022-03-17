#include <iostream>
#include <DirectXColors.h>
#include "BaseApp.h"
#include "D3D/d3dutil.h"
#include "Exception/ExceptionBase.h"
#include "GameTimer/GameTimer.h"
#include "dx12lib/Device.h"
#include "dx12lib/CommandListProxy.h"
#include "dx12lib/CommandList.h"
#include "dx12lib/CommandQueue.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/RenderTarget.h"
#include "dx12lib/Texture.h"


class TestApp : public com::BaseApp {
public:
	TestApp() = default;
	virtual void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
};

void TestApp::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pCmdList = pCmdQueue->createCommandListProxy();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	pCmdList->setViewports(pRenderTarget->getViewport());
	pCmdList->setScissorRects(pRenderTarget->getScissiorRect());

	{
		dx12lib::RenderTargetTransitionBarrier barrierGuard = {
			pCmdList,
			pRenderTarget,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
		};

		auto pRenderTargetBuffer = pRenderTarget->getRenderTargetBuffer(dx12lib::Color0);
		auto pDepthStencilBuffer = pRenderTarget->getDepthStencilBuffer();
		pCmdList->clearColor(pRenderTargetBuffer, { 1.f, std::sin(pGameTimer->getTotalTime()) * 0.5f + 0.5f, 0.f, 1.f });
		pCmdList->clearDepthStencil(pDepthStencilBuffer, 1.f, 0);
		pCmdList->setRenderTarget(pRenderTarget);
	}
	pCmdQueue->executeCommandList(pCmdList);
	pCmdQueue->signal(_pSwapChain);
}

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	try {
		TestApp app;
		app.initialize();
		while (app.isRuning()) {
			pGameTimer->newFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
		app.destory();
	} catch (const com::ExceptionBase &e) {
		std::cerr << e.what() << std::endl;
		OutputDebugStringA(e.what());
	}
	return 0;
}