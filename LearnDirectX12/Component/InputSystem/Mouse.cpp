#include "Mouse.h"
#include <iostream>
#include "Window.h"
#include "InputSystem.h"

namespace com {

Mouse::Mouse(InputSystem *pInputSystem) : _pInputSystem(pInputSystem) {
	updateWindowCenter();
}

void Mouse::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	bool isEvent = true;
	MouseEvent mouseEvent = { x, y, MouseState::Invalid, 0.f };
	switch (msg) {
	case WM_LBUTTONDOWN:
		mouseEvent.state_ = MouseState::LPress;
		break;
	case WM_LBUTTONUP:
		mouseEvent.state_ = MouseState::LRelease;
		break;
	case WM_RBUTTONDOWN:
		mouseEvent.state_ = MouseState::RPress;
		break;
	case WM_RBUTTONUP:
		mouseEvent.state_ = MouseState::RRelease;
		break;
	case WM_MOUSEMOVE:
		mouseEvent.state_ = MouseState::Move;
		if (!_showCursor && x == _windowCenter.x && y == _windowCenter.y) {
			// todo: 这里应该给外部停供虚假的 xy 坐标. 在 window 里面会调用 SetCursorPos 函数
			// todo: 外面计算出来的 dx 和 dy 是错误的
			isEvent = false;
		}
		break;
	case WM_MBUTTONDOWN:
		mouseEvent.state_ = MouseState::WheelDown;
		break;
	case WM_MBUTTONUP:
		mouseEvent.state_ = MouseState::WheelUp;
		break;
	case WM_MOUSEWHEEL:
		mouseEvent.offset_ = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		mouseEvent.state_ = MouseState::Wheel;
		break;
	default:
		isEvent = false;
		break;
	}
	if (isEvent)
		events_.push(mouseEvent);
}

void Mouse::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	while (events_.size() > EventMaxSize_)
		events_.pop();
}

MouseEvent Mouse::getEvent() {
	if (events_.empty())
		return {};

	auto res = events_.front();
	events_.pop();
	return res;
}

bool Mouse::getShowCursor() const {
	return _showCursor;
}

void Mouse::setShowCursor(bool bShow) {
	if (_showCursor != bShow) {
		_showCursor = bShow;
		setShowCursor(bShow);
	}

	if (!bShow)
		updateWindowCenter();
}

void Mouse::updateWindowCenter() {
	RECT rect;
	GetWindowRect(_pInputSystem->window->getHWND(), &rect);
	_windowCenter.x = (rect.right + rect.left) / 2;
	_windowCenter.y = (rect.bottom + rect.top) / 2;
	ScreenToClient(_pInputSystem->window->getHWND(), &_windowCenter);
	ClipCursor(&rect);
}

MouseEvent::operator bool() const {
	return !isInvalid();
}

}
