#include "InputSystem.h"

InputSystem::InputSystem(const std::string &title, int width, int height) {
	window = std::make_unique<Window>(width, height, title);
	mouse = std::make_unique<Mouse>();
	keyboard = std::make_unique<Keyboard>();
	window->setMessageCallback([&](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		mouse->handleMsg(hwnd, msg, wParam, lParam);
		keyboard->handleMsg(hwnd, msg, wParam, lParam);
	});
}

bool InputSystem::shouldClose() const {
	return window->shouldClose();
}

void InputSystem::tick(GameTimer &gt) {
	window->pollEvent();
	window->tick(gt);
	mouse->tick(gt);
	keyboard->tick(gt);
}
