#pragma once
#include <queue>
#include <windows.h>
#include <bitset>
#include "ITick.h"

class Mouse : public ITick {
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
		int		x;
		int		y;
		State	state_;
		float	offset_;
	};
	Mouse();
	Mouse(const Mouse &) = delete;
	Mouse &operator=(const Mouse &) = delete;
	~Mouse() = default;
	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void beginTick() override;
	virtual void tick(GameTimer &dt) override;
	virtual void endTick() override;
private:
	std::queue<Event>		events_;
	std::bitset<MaxCount>	state_;
};

