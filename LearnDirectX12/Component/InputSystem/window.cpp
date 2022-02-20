#include <iostream>
#include <format>
#include "Window.h"
#include <errhandlingapi.h>
#include <cassert>
#include "GameTimer/GameTimer.h"

namespace com {

Window::Window(int width, int height, const std::string &title)
	: hwnd_(nullptr), width_(width), height_(height), title_(title)
	, shouldClose_(false), result(-1)
{
	resizeCallback_ = [](int, int) {};
	messageCallback_ = [](HWND, UINT, WPARAM, LPARAM) {};

	RECT wr;
	wr.left = 100;
	wr.right = wr.left + width;
	wr.top = 100;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	hwnd_ = CreateWindowEx(
		0, WindowClass::getClassName(), title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::getInstance(), this
	);

	assert(hwnd_ != nullptr && "hwnd is nullptr");
	ShowWindow(hwnd_, SW_SHOWDEFAULT);
	shouldClose_ = false;
}

bool Window::shouldClose() const {
	return shouldClose_;
}

void Window::setShouldClose(bool flag) {
	shouldClose_ = flag;
}

int Window::getReturnCode() const {
	return result;
}

void Window::setMessageCallback(const std::function<void(HWND, UINT, WPARAM, LPARAM)> &callback) {
	messageCallback_ = callback;
}

void Window::setResizeCallback(const std::function<void(int x, int y)> &callback) {
	resizeCallback_ = callback;
}

int Window::getWidth() const {
	return width_;
}

int Window::getHeight() const {
	return height_;
}

HWND Window::getHWND() const {
	return hwnd_;
}

float Window::aspectRatio() const {
	return float(width_) / float(height_);
}

const std::string & Window::getTitle() const {
	return title_;
}

void Window::setShowTitle(const std::string &title) {
	title_ = title;
}

bool Window::isPause() const {
	return paused_;
}

void Window::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	pGameTimer_ = pGameTimer;
	MSG msg;
	BOOL getResult = 0;
	while (PeekMessage(&msg, nullptr, 0, 0, true)) {
		if (msg.message == WM_QUIT) {
			shouldClose_ = true;
			result = static_cast<int>(msg.wParam);
			return;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (resizeDirty_) {
		resizeDirty_ = false;
		resizeCallback_(width_, height_);
	}
	if (pGameTimer->oneSecondTrigger()) {
		std::stringstream sbuf;
		sbuf << title_ << ' ';
		sbuf << "fps: " << pGameTimer->FPS() << ' ';
		sbuf << "mspf: " << pGameTimer->mspf() << "ms";
		auto title = sbuf.str();
		SetWindowText(hwnd_, title.c_str());
	}
}

Window::~Window() {
	DestroyWindow(hwnd_);
}

LRESULT CALLBACK Window::handleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NCCREATE) {
		const CREATESTRUCT *pCreate = reinterpret_cast<const CREATESTRUCT *>(lParam);
		Window *pWindow = static_cast<Window *>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&handleMsgThunk));
		return pWindow->handleMsg(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::handleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Window *ptr = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (ptr != nullptr)
		return ptr->handleMsg(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// set virtual code https://docs.microsoft.com/zh-cn/windows/win32/inputdev/virtual-key-codes
LRESULT Window::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
	case WM_CLOSE:
	{
		PostQuitMessage(1);		// set exit application code
		shouldClose_ = true;
		break;
	}
	case WM_PAINT:
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);
		width_ = rect.right - rect.left;
		height_ = rect.bottom - rect.top;
		resizeDirty_ = true;
		break;
	}
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			stop();
		else
			start();
		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		resizing_ = true;
		stop();
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		resizing_ = false;
		start();
		break;
	}
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
		break;
	}
	case WM_SIZE:
	{
		width_ = LOWORD(lParam);
		height_ = HIWORD(lParam);
		if (wParam == SIZE_MINIMIZED) {
			paused_ = true;
			minimized_ = true;
			maximized_ = false;
		} else if (wParam == SIZE_MAXIMIZED) {
			paused_ = false;
			minimized_ = false;
			maximized_ = true;
			resizeDirty_ = true;
		} else if (wParam == SIZE_RESTORED) {		// restore for old state
			if (minimized_) {
				paused_ = false;
				minimized_ = false;
				resizeDirty_ = true; 
			} else if (maximized_) {
				paused_ = false;
				maximized_ = false;
				resizeDirty_ = true;
			} else if (resizing_) {
				break;
			} else {
				resizeDirty_ = true;
			}
		}
		break;
	}
	}
	messageCallback_(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void Window::stop() {
	paused_ = true;
	if (pGameTimer_ != nullptr)
		pGameTimer_->stop();
}


void Window::start() {
	paused_ = false;
	if (pGameTimer_ != nullptr)
		pGameTimer_->start();
}

Window::WindowClass::WindowClass() : hInstance_(GetModuleHandle(nullptr)) {
	// register class
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = handleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance_;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = nullptr; //;tatic_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = getClassName();
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
}

const char *Window::WindowClass::getClassName() {
	return "WindowClass";
}

HINSTANCE Window::WindowClass::getInstance() {
	return singletonPtr_->hInstance_;
}

}