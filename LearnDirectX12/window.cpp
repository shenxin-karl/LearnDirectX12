#include <iostream>
#include <format>
#include <windows.h>
#include <errhandlingapi.h>
#include "Window.h"

Window::Window(int width, int height, const std::string &title)
: hwnd_(nullptr), width_(width), height_(height)
, shouldClose_(false), result(-1) {
	resizeCallback_ = [](int, int) {};
	messageCallback_ = [](HWND, UINT, WPARAM, LPARAM) {};

	RECT wr;
	wr.left = 100;
	wr.right = wr.left + width;
	wr.top = 100;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
	hwnd_ = CreateWindowEx(
		0, WindowClass::getClassName(), title.c_str(),
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::getInstance(), this
	);

	SAssert(hwnd_ != nullptr, "hwnd is nullptr");
	ShowWindow(hwnd_, SW_SHOWDEFAULT);
	shouldClose_ = false;
}

void Window::pollEvent() {
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
		resizeCallback_(width_, height_);
		break;
	}
	}
	messageCallback_(hwnd, msg, wParam, lParam);
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

Window::WindowClass::WindowClass() : hInstance_(GetModuleHandle(nullptr)) {
	// register class
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = handleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance_;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
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

void CheckWindowErrorImpl(HRESULT hr, const char *file, int line) {
	if (hr != S_OK) {
		char *pMsgBuf = nullptr;
		DWORD nMsgLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr);

		std::string errMsg = std::format("[{}]: {}", hr, (nMsgLen == 0 ? "unidentifyied error code" : pMsgBuf));
		LocalFree(pMsgBuf);
		SAssertImpl(false, errMsg, file, line);
	}
}