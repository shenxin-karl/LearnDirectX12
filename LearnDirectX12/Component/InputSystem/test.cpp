#include <iostream>
#include "InputSystem.h"

int main() {
	std::unique_ptr<InputSystem> pInputSystem = std::make_unique<InputSystem>("Title", 800, 600);
}