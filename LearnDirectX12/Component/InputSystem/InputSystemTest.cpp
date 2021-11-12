#include <iostream>
#include "Component/GameTimer/GameTimer.h"
#include "InputSystem.h"


int main() {
	com::GameTimer gt;
	std::unique_ptr<com::InputSystem> pInputSystem = std::make_unique<com::InputSystem>("Title", 800, 600);
	while (!pInputSystem->shouldClose()) {
		pInputSystem->tick(gt);
	}
}