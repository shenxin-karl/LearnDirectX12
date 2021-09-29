#pragma once
#include <queue>
#define NOMINMAX
#include <Windows.h>
#include <bitset>

class Window;
class Mouse {
public:
	enum State {
		LPress,
		LRelease,
		RPress,
		RRelease,
		WheelDown,
		WheelUp,
		Move,
		Wheel,
		Invalid,
		MaxCount,
	};

	struct Event {
		bool isLPress() const { return state_ == LPress; }
		bool isLRelease() const { return state_ == LRelease; }
		bool isRPress() const { return state_ == RPress; }
		bool isRRelease() const { return state_ == RRelease; }
		bool isWheelDown() const { return state_ == WheelDown; }
		bool isWheelUp() const { return state_ == WheelUp; }
		bool isMove() const { return state_ == Move; }
		bool isWheel() const { return state_ == Wheel; }
		bool isInvalid() const { return state_ == Invalid; }
	public:
		int		x;
		int		y;
		State	state_;
		float	offset_;
	};

	Mouse(Window *window);
	Mouse(const Mouse &) = delete;
	Mouse &operator=(const Mouse &) = delete;
	~Mouse() = default;
	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	Window				   *window_;
	std::queue<Event>		events_;
	std::bitset<MaxCount>	state_;
};

