#pragma once
#include <memory>
#include <string>
#include "Window.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Component/ITick.h"

namespace com {

class InputSystem : public ITick {
public:
	InputSystem(const std::string &title, int width, int height);
	InputSystem(const InputSystem &) = delete;
	InputSystem &operator=(const InputSystem &) = delete;
	InputSystem() = default;
	bool shouldClose() const;
	virtual void tick(GameTimer &gt) override;
public:
	std::unique_ptr<Mouse>		mouse;
	std::unique_ptr<Keyboard>	keyboard;
	std::unique_ptr<Window>		window;
};

}