#pragma once
#include <windows.h>
#include <memory>
#include <string>
#include <functional>

namespace com {

class GameTimer;
class Window {
	class WindowClass;
public:
	Window(int width, int height, const std::string &title);
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	void pollEvent();
	bool shouldClose() const;
	void setShouldClose(bool flag);
	int getReturnCode() const;
	void setMessageCallback(const std::function<void(HWND, UINT, WPARAM, LPARAM)> &callback);
	void setResizeCallback(const std::function<void(int x, int y)> &callback);
	int getWidth() const;
	int getHeight() const;
	HWND getHWND() const;
	float aspectRatio() const;
	const std::string &getTitle() const;
	void setShowTitle(const std::string &title);
	bool isPause() const;
	void tick(GameTimer &gt);
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
	std::string title_;
	std::function<void(HWND, UINT, WPARAM, LPARAM)> messageCallback_;
	std::function<void(int x, int y)>				resizeCallback_;
	GameTimer									   *pGameTimer;
public:
	bool paused_ = false;
	bool minimized_ = false;
	bool maximized_ = false;
	bool resizing_ = false;
	bool fullScreenState_ = false;
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

void CheckWindowErrorImpl(HRESULT hr, const char *file, int line);
#define CheckWindowError(hr) CheckWindowErrorImpl(hr, __FILE__, __LINE__)
#define CheckWindowLastError() CheckWindowError(GetLastError())

}