#include "BaseApp.h"
#include "InputSystem/Window.h"
#include "dx12lib/Adapter.h"
#include "dx12lib/Device.h"
#include "dx12lib/SwapChain.h"
#include "dx12lib/CommandQueue.h"

namespace com {

void BaseApp::initialize() {
	_pInputSystem = std::make_unique<InputSystem>(_title, _width, _height);
	_pInputSystem->window->setResizeCallback([&](int width, int height) {
		resize(width, height);
	});

	_pAdapter = std::make_shared<dx12lib::Adapter>();
	_pDevice = std::make_shared<dx12lib::Device>(_pAdapter);
	_pDevice->initialize();
	_pSwapChain = _pDevice->createSwapChain(_pInputSystem->window->getHWND(), _backBufferFormat, _depthStencilFormat);
	// first resize
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	pCmdQueue->resize(_width, _height, _pSwapChain);
	onInitialize();
}

void BaseApp::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	_pInputSystem->beginTick(pGameTimer);
	if (_pInputSystem->window->isPause()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(1000.f / 60.f)));
		return;
	}

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
	if (width == _width || height == _height || width == 0 || height == 0)
		return;

	_width = width;
	_height = height;
	auto pCmdQueue = _pDevice->getCommandQueue(dx12lib::CommandQueueType::Direct);
	pCmdQueue->resize(width, height, _pSwapChain);
}

bool BaseApp::isRuning() const {
	return !_pInputSystem->shouldClose();
}

}