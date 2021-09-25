#pragma once
#define NOMINMAX       
#include <Windows.h>
#include <memory>
#include <string>
#include "CommonDefine.h"

class Window {
	class WindowClass;
public:
	Window(int width, int height, const std::string &title);
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	void pollEvent();
	bool shouldClose() const;
	int getReturnCode() const;
	~Window();
private:
	static LRESULT CALLBACK handleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK handleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	HWND hwnd_;
	int	 width_;
	int	 height_;
	bool shouldClose_;
	int  result;
};

class Window::WindowClass {
	HINSTANCE hInstance_ = nullptr;
private:
	static inline std::unique_ptr<WindowClass> singletonPtr_ = std::make_unique<WindowClass>();
public:
	WindowClass();
	static HINSTANCE getInstance();
	static const char *getClassName();
};