#include "Mouse.h"

namespace com {

Mouse::Mouse() {
}

void Mouse::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	bool isEvent = true;
	MouseEvent mouseEvent ={ x, y, MouseState::Invalid, 0.f };
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

MouseEvent::operator bool() const {
	return !isInvalid();
}

}