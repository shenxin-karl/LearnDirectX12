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
	TestApp();
	virtual void beginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
};

TestApp::TestApp() {

}

void TestApp::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	pCmdQueue->newFrame();
}

void TestApp::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pCmdList = pCmdQueue->createCommandListProxy();
	auto pRenderTarget = _pSwapChain->getRenderTarget();
	auto pTexture = pRenderTarget->getTexture(dx12lib::AttachmentPoint::Color0);
	pTexture->clearColorDepthStencil(
		{ 1.f, std::sin(pGameTimer->getTotalTime()) * 0.5f + 0.5f, 0.f, 1.f }, 
		1.f, 
		1
	);

	pRenderTarget->setRTVClearValueDirty(true);
	pRenderTarget->setDSVClearValueDirty(true);
	pCmdList->setRenderTarget(pRenderTarget);

	pCmdQueue->executeCommandList(pCmdList);
	pCmdQueue->signal(_pSwapChain);
}

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	try
	{
		TestApp app;
		app.initialize();
		while (app.isRuning()) {
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
	} catch (const com::ExceptionBase &e) {
		std::cerr << e.what() << std::endl;
		OutputDebugStringA(e.what());
	}
	return 0;
}