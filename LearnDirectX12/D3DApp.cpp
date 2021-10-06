#include "D3DApp.h"
#include "GameTimer.h"
#include "window.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Graphics.h"

D3DApp::D3DApp() {}

void D3DApp::initialize() {
	timerPtr_ = std::make_unique<GameTimer>();
	windowPtr_ = std::make_unique<Window>(800, 600, "LearnDirectX");
	keyboardPtr_ = std::make_unique<Keyboard>();
	mousePtr_ = std::make_unique<Mouse>();
	graphicsPtr_ = std::make_unique<Graphics>();
	graphicsPtr_->initialize();

	windowPtr_->setMessageCallback([this](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		keyboardPtr_->handleMsg(hwnd, msg, wParam, lParam);
		mousePtr_->handleMsg(hwnd, msg, wParam, lParam);
		graphicsPtr_->handleMsg(hwnd, msg, wParam, lParam);
	});
}

int D3DApp::start() {
	try {
		initialize();
		return run();
	} catch (const ExceptionBase &e) {
		MessageBox(nullptr, e.what(), e.getType(), MB_OK | MB_ICONEXCLAMATION);
	} catch (const std::exception &e) {
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONEXCLAMATION);
	} catch (...) {
		MessageBox(nullptr, "Undefined error", "Error", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}

int D3DApp::run() {

	while (!windowPtr_->shouldClose()) {
		windowPtr_->pollEvent();

		// being Tick
		windowPtr_->beginTick();
		keyboardPtr_->beginTick();
		mousePtr_->beginTick();
		graphicsPtr_->beginTick();

		// tick
		timerPtr_->tick();
		windowPtr_->tick();
		keyboardPtr_->tick();
		mousePtr_->tick();
		graphicsPtr_->tick();

		// endTick
		windowPtr_->endTick();
		keyboardPtr_->endTick();
		mousePtr_->endTick();
		graphicsPtr_->endTick();
	}
	return windowPtr_->getReturnCode();
}

D3DApp::~D3DApp() {
	return;
}
