#include "GameTimer/GameTimer.h"
#include "InputSystem.h"
#include "Window.h"
#include "Mouse.h"
#include "Keyboard.h"

namespace com {

InputSystem::InputSystem(const std::string &title, int width, int height) {
	window = std::make_unique<Window>(width, height, title);
	mouse = std::make_unique<Mouse>();
	keyboard = std::make_unique<Keyboard>();
	window->setMessageCallback([&](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		mouse->handleMsg(hwnd, msg, wParam, lParam);
		keyboard->handleMsg(hwnd, msg, wParam, lParam);
	});
}

InputSystem::~InputSystem() {
}

bool InputSystem::shouldClose() const {
	return window->shouldClose();
}


void InputSystem::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	window->beginTick(pGameTimer);
	mouse->beginTick(pGameTimer);
	keyboard->beginTick(pGameTimer);
}

void InputSystem::tick(std::shared_ptr<GameTimer> pGameTimer) {
	window->tick(pGameTimer);
	mouse->tick(pGameTimer);
	keyboard->tick(pGameTimer);
}


void InputSystem::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	window->endTick(pGameTimer);
	mouse->endTick(pGameTimer);
	keyboard->endTick(pGameTimer);
}

}