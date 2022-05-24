#include <iostream>
#include <format>
#include <cassert>
#include "Window.h"
#include "InputSystem.h"
#include "Mouse.h"
#include "GameTimer/GameTimer.h"

namespace com {

Window::Window(int width, int height, const std::string &title, InputSystem *pInputSystem)
	: _hwnd(nullptr), _width(width), _height(height), _title(title)
	, _shouldClose(false), result(-1), _pInputSystem(pInputSystem)
{
	_resizeCallback = [](int, int) {};
	_messageDispatchCallback = [](HWND, UINT, WPARAM, LPARAM) {};

	RECT wr;
	wr.left = 100;
	wr.right = wr.left + width;
	wr.top = 100;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	_hwnd = CreateWindowEx(
		0, WindowClass::getClassName(), title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::getInstance(), this
	);

	assert(_hwnd != nullptr && "hwnd is nullptr");
	ShowWindow(_hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(_hwnd);
	_shouldClose = false;

	// Show the window
}

bool Window::shouldClose() const {
	return _shouldClose;
}

void Window::setShouldClose(bool flag) {
	_shouldClose = flag;
}

int Window::getReturnCode() const {
	return result;
}

void Window::setMessageDispatchCallback(const std::function<void(HWND, UINT, WPARAM, LPARAM)> &callback) {
	_messageDispatchCallback = callback;
}

void Window::setPrepareMessageCallBack(const std::function<bool(HWND, UINT, WPARAM, LPARAM)> &callback) {
	_prepareMessageCallBack = callback;
}

void Window::setResizeCallback(const std::function<void(int x, int y)> &callback) {
	_resizeCallback = callback;
}

int Window::getWidth() const {
	return _width;
}

int Window::getHeight() const {
	return _height;
}

HWND Window::getHWND() const {
	return _hwnd;
}

float Window::aspectRatio() const {
	return float(_width) / float(_height);
}

const std::string & Window::getTitle() const {
	return _title;
}

void Window::setShowTitle(const std::string &title) {
	_title = title;
}

bool Window::isPause() const {
	return _paused;
}

void Window::beginTick(std::shared_ptr<GameTimer> pGameTimer) {
	_pGameTimer = pGameTimer;
	MSG msg;
	BOOL getResult = 0;
	while (PeekMessage(&msg, nullptr, 0, 0, true)) {
		if (msg.message == WM_QUIT) {
			_shouldClose = true;
			result = static_cast<int>(msg.wParam);
			return;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (_resizeDirty) {
		_resizeDirty = false;
		_resizeCallback(_width, _height);
	}

	if (!_pInputSystem->pMouse->getShowCursor())
		_pInputSystem->pMouse->adjustCursorPosition();

	if (pGameTimer->oneSecondTrigger()) {
		std::stringstream sbuf;
		sbuf << _title << ' ';
		sbuf << "fps: " << pGameTimer->FPS() << ' ';
		sbuf << "mspf: " << pGameTimer->mspf() << "ms";
		auto title = sbuf.str();
		SetWindowText(_hwnd, title.c_str());
	}
}

Window::~Window() {
	DestroyWindow(_hwnd);
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
	if (_prepareMessageCallBack != nullptr && _prepareMessageCallBack(hwnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_DESTROY:
	case WM_CLOSE:
	{
		PostQuitMessage(1);		// set exit application code
		_shouldClose = true;
		break;
	}
	case WM_PAINT:
	{
		RECT rect;
		GetWindowRect(hwnd, &rect);
		_width = rect.right - rect.left;
		_height = rect.bottom - rect.top;
		_resizeDirty = true;
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
		_resizing = true;
		stop();
		break;
	}
	case WM_EXITSIZEMOVE:
	{
		_resizing = false;
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
		_width = LOWORD(lParam);
		_height = HIWORD(lParam);
		if (wParam == SIZE_MINIMIZED) {
			_paused = true;
			_minimized = true;
			_maximized = false;
		} else if (wParam == SIZE_MAXIMIZED) {
			_paused = false;
			_minimized = false;
			_maximized = true;
			_resizeDirty = true;
		} else if (wParam == SIZE_RESTORED) {		// restore for old state
			if (_minimized) {
				_paused = false;
				_minimized = false;
				_resizeDirty = true; 
			} else if (_maximized) {
				_paused = false;
				_maximized = false;
				_resizeDirty = true;
			} else if (_resizing) {
				break;
			} else {
				_resizeDirty = true;
			}
		}
		break;
	}
	}
	_messageDispatchCallback(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


void Window::stop() {
	_paused = true;
	if (_pGameTimer != nullptr)
		_pGameTimer->stop();
}


void Window::start() {
	_paused = false;
	if (_pGameTimer != nullptr)
		_pGameTimer->start();
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
	return _pSingleton->hInstance_;
}

}