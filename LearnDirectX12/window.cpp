#include <iostream>
#include <format>
#include "window.h"

Window::Window(int width, int height, const std::string &title)
: hwnd_(nullptr), width_(width), height_(height)
, shouldClose_(false), result(-1) {
	
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
	while ((getResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (getResult == -1) {
		result = -1;
		shouldClose_ = true;
	} else {
		result = static_cast<int>(msg.wParam);
	}
}

bool Window::shouldClose() const {
	return shouldClose_;
}

int Window::getReturnCode() const {
	return result;
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
	static std::string title;
	switch (msg) {
	case WM_CLOSE:
	{
		PostQuitMessage(69);		// set exit application code
		shouldClose_ = true;
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
			title.clear();
		break;
	}
	case WM_CHAR:
	{
		title.push_back(static_cast<char>(wParam));
		SetWindowText(hwnd, title.c_str());
		break;
	}
	case WM_LBUTTONDOWN:
	{
		POINTS pt = MAKEPOINTS(lParam);
		std::string location = std::format("({}, {})", pt.x, pt.y);
		SetWindowText(hwnd, location.c_str());
		title.clear();
		break;
	}
	}
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
