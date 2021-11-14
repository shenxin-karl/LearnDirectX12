#include <iostream>
#include <format>
#include "GameTimer/GameTimer.h"
#include "InputSystem.h"
#include "Window.h"
#include "Mouse.h"
#include "Keyboard.h"

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_unique<com::GameTimer>();
	std::unique_ptr<com::InputSystem> pInputSystem = std::make_unique<com::InputSystem>("Title", 800, 600);
	while (!pInputSystem->shouldClose()) {
		pInputSystem->tick(pGameTimer);
		while (auto charEvent = pInputSystem->keyboard->readChar())
			std::cout << charEvent.getCharacter() << std::endl;
		while (auto mouseEvent = pInputSystem->mouse->getEvent()) {
			switch (mouseEvent.state_) {
			case com::Mouse::State::LPress:
				std::cout << std::format("LPress:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::LRelease:
				std::cout << std::format("LRelease:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::RPress:
				std::cout << std::format("RPress:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::RRelease:
				std::cout << std::format("RRelease:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::Move:
				std::cout << std::format("Move:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::Wheel:
				std::cout << std::format("Wheel:({})", mouseEvent.offset_) << std::endl;
				break;
			case com::Mouse::State::WheelDown:
				std::cout << std::format("WheelDown:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			case com::Mouse::State::WheelUp:
				std::cout << std::format("WheelUp:({}, {})", mouseEvent.x, mouseEvent.y) << std::endl;
				break;
			}
		}
	}
	return 0;
}