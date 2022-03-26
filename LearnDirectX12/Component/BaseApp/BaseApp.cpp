#include "BaseApp.h"
#include "InputSystem/window.h"
#include "dx12lib/Adapter.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/CommandQueue.h"

namespace com {

void BaseApp::initialize() {
	_pInputSystem = std::make_unique<InputSystem>(_title, _width, _height);
	_pInputSystem->initialize();
	_pInputSystem->window->setResizeCallback([&](int width, int height) {
		resize(width, height);
	});

	_pAdapter = std::make_shared<dx12lib::Adapter>();
	_pDevice = std::make_shared<dx12lib::Device>(_pAdapter);

	dx12lib::DeviceInitDesc desc = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
	};
	_pDevice->initialize(desc);
	_pSwapChain = _pDevice->createSwapChain(_pInputSystem->window->getHWND());
	// first resize
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	auto pDirectContext = pCmdQueue->createDirectContextProxy();
	_pSwapChain->resize(pDirectContext, _width, _height);
	onInitialize(pDirectContext);
	pCmdQueue->executeCommandList(pDirectContext);
	pCmdQueue->signal(_pSwapChain);
	pCmdQueue->flushCommandQueue();
}

void BaseApp::destory() {
	onDistory();
	_pInputSystem->destory();
	_pDevice->destory();
}

void BaseApp::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	_pInputSystem->beginTick(pGameTimer);
	if (_pInputSystem->window->isPause()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000.f / 60.f)));
		return;
	}

	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	pCmdQueue->newFrame();		// start new frames
	onBeginTick(pGameTimer);
}

void BaseApp::tick(std::shared_ptr<GameTimer> pGameTimer) {
	_pInputSystem->tick(pGameTimer);
	if (_pInputSystem->window->isPause())
		return;

	onTick(pGameTimer);
}

void BaseApp::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	_pInputSystem->endTick(pGameTimer);
	if (_pInputSystem->window->isPause())
		return;

	_pDevice->releaseStaleDescriptor();
	onEndTick(pGameTimer);
}

void BaseApp::resize(int width, int height) {
	if (width == 0 || height == 0)
		return;

	_width = width;
	_height = height;
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	pCmdQueue->flushCommandQueue();
	pCmdQueue->newFrame();
	auto pCmdList = pCmdQueue->createDirectContextProxy();
	_pSwapChain->resize(pCmdList, width, height);
	onResize(pCmdList, width, height);
	pCmdQueue->executeCommandList(pCmdList);
	pCmdQueue->signal(_pSwapChain);
	pCmdQueue->flushCommandQueue();
}

bool BaseApp::isRuning() const {
	return !_pInputSystem->shouldClose();
}

}