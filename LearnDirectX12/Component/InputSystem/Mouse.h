#pragma once
#include <queue>
#include <windows.h>
#include <bitset>

namespace com {

class GameTimer;
class Mouse {
public:
	static constexpr size_t EventMaxSize_ = 0xff;
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
		int		x = 0;
		int		y = 0;
		State	state_ = Invalid;
		float	offset_ = 0;
	};
	Mouse();
	Mouse(const Mouse &) = delete;
	Mouse &operator=(const Mouse &) = delete;
	~Mouse() = default;
	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void tick(GameTimer &gt);
	Event getEvent();
private:
	std::queue<Event>		events_;
};

}