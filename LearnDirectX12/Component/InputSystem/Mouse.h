#pragma once
#include <queue>
#include <windows.h>
#include <bitset>
#include "ITick.h"

namespace com {

class GameTimer;
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
		bool isLPress() const { return state_ == State::LPress; }
		bool isLRelease() const { return state_ == State::LRelease; }
		bool isRPress() const { return state_ == State::RPress; }
		bool isRRelease() const { return state_ == State::RRelease; }
		bool isWheelDown() const { return state_ == State::WheelDown; }
		bool isWheelUp() const { return state_ == State::WheelUp; }
		bool isMove() const { return state_ == State::Move; }
		bool isWheel() const { return state_ == State::Wheel; }
		bool isInvalid() const { return state_ == State::Invalid; }
		explicit operator bool() const;
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
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
	Event getEvent();
private:
	std::queue<Event>		events_;
};

}