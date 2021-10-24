#pragma once
#define NOMINMAX
#include <windows.h>
#include "Singleton.hpp"


class Graphics;
class Window;
class Keyboard;
class Mouse;
class GameTimer;
class AppControl;
class D3DApp : public Singleton<D3DApp> {
public:
	D3DApp(std::shared_ptr<AppControl> controlPtr);
	D3DApp(const D3DApp &) = delete;
	D3DApp &operator=(const D3DApp &) = delete;
	void initialize();
	int start();
	int run();
	~D3DApp();
public:
	GameTimer *getGameTimer() noexcept { return timerPtr_.get(); }
	Window *getWindow() noexcept { return windowPtr_.get(); }
	Keyboard *getKeyboard() noexcept { return keyboardPtr_.get(); }
	Mouse *getMouse() noexcept { return mousePtr_.get(); }
	Graphics *getGraphics() noexcept { return graphicsPtr_.get(); }
private:
	std::unique_ptr<GameTimer>	timerPtr_;
	std::unique_ptr<Window>		windowPtr_;
	std::unique_ptr<Keyboard>	keyboardPtr_;
	std::unique_ptr<Mouse>		mousePtr_;
	std::unique_ptr<Graphics>	graphicsPtr_;
	std::shared_ptr<AppControl> appControlPtr_;
};

