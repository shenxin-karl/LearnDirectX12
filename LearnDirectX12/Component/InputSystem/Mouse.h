#pragma once
#define  NOMINMAX
#include <queue>
#include <windows.h>
#include <bitset>
#include "ITick.h"

namespace com {

enum class MouseState : int {
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

struct MouseEvent {
	bool isLPress() const { return state_ == MouseState::LPress; }
	bool isLRelease() const { return state_ == MouseState::LRelease; }
	bool isRPress() const { return state_ == MouseState::RPress; }
	bool isRRelease() const { return state_ == MouseState::RRelease; }
	bool isWheelDown() const { return state_ == MouseState::WheelDown; }
	bool isWheelUp() const { return state_ == MouseState::WheelUp; }
	bool isMove() const { return state_ == MouseState::Move; }
	bool isWheel() const { return state_ == MouseState::Wheel; }
	bool isInvalid() const { return state_ == MouseState::Invalid; }
	explicit operator bool() const;
public:
	int		    x = 0;
	int		    y = 0;
	MouseState	state_ = MouseState::Invalid;
	float	    offset_ = 0;
};

class GameTimer;
class Mouse : public ITick {
public:
	Mouse();
	Mouse(const Mouse &) = delete;
	Mouse &operator=(const Mouse &) = delete;
	~Mouse() = default;
	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
	MouseEvent getEvent();
private:
	static constexpr size_t EventMaxSize_ = 0xff;
	std::queue<MouseEvent> events_;
};

}