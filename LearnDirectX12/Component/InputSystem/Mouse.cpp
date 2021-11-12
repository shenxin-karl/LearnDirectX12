#include "Mouse.h"

namespace com {

Mouse::Mouse() {
}

void Mouse::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	bool isEvent = true;
	Event mouseEvent ={ x, y, Invalid, 0.f };
	switch (msg) {
	case WM_LBUTTONDOWN:
		mouseEvent.state_ = LPress;
		break;
	case WM_LBUTTONUP:
		mouseEvent.state_ = LRelease;
		break;
	case WM_RBUTTONDOWN:
		mouseEvent.state_ = RPress;
		break;
	case WM_RBUTTONUP:
		mouseEvent.state_ = RRelease;
		break;
	case WM_MOUSEMOVE:
		mouseEvent.state_ = Move;
		break;
	case WM_MBUTTONDOWN:
		mouseEvent.state_ = WheelDown;
		break;
	case WM_MBUTTONUP:
		mouseEvent.state_ = WheelUp;
		break;
	case WM_MOUSEWHEEL:
		mouseEvent.offset_ = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		mouseEvent.state_ = Wheel;
		break;
	default:
		isEvent = false;
		break;
	}
	if (isEvent)
		events_.push(mouseEvent);
}


void Mouse::tick(GameTimer &gt) {
	while (events_.size() > EventMaxSize_)
		events_.pop();
}

Mouse::Event Mouse::getEvent() {
	if (events_.empty())
		return {};

	auto res = events_.front();
	events_.pop();
	return res;
}

}