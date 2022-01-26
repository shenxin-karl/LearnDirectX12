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


class TestApp : public com::BaseApp {
public:
	TestApp();
	virtual void tick(std::shared_ptr<com::GameTimer> pGameTimer) override;
};

TestApp::TestApp() {

}

void TestApp::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pCmdList = pCmdQueue->createCommandListProxy();

	//pCmdQueue->executeCommandList(pCmdList)
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