#pragma once
#define NOMINMAX
#include <windows.h>
#include <bitset>
#include <queue>
#include "ITick.h"

namespace com {

	enum class KeyState : int {
		Pressed,
		Released,
		Invalid,
	};

	struct CharEvent {
		CharEvent() = default;
		CharEvent(KeyState state, unsigned char character);
		unsigned char getCharacter() const;
		KeyState getState() const;
		bool isPressed() const;
		bool isInvalid() const;
		explicit operator bool() const;
	private:
		KeyState		state_ = KeyState::Invalid;
		unsigned char	character_ = 0;
	};

	struct KeyEvent {
		KeyEvent() = default;
		KeyEvent(KeyState state, unsigned char key);
		unsigned char getKey() const;
		KeyState getState() const;
		bool isPressed() const;
		bool isReleased() const;
		bool isInvalid() const;
		explicit operator bool() const;
	private:
		KeyState      state_ = KeyState::Invalid;
		unsigned char key_ = 0;				// Use the window's virtual button
	};

class GameTimer;
class Keyboard : public ITick {

public:
	static constexpr int MaxKeyCodeSize_ = 0xff;
	static constexpr int MaxQueueSize_ = 0xff;
	std::bitset<MaxKeyCodeSize_>	keyState_;
	std::bitset<MaxKeyCodeSize_>	characterState_;
	std::queue<KeyEvent>			keycodeQueue_;
	std::queue<CharEvent>			characterQueue_;
public:
	Keyboard();
	Keyboard(const Keyboard &) = delete;
	Keyboard &operator=(const Keyboard &) = delete;
	bool isKeyPressed(unsigned char key) const;
	bool isCharPressed(unsigned char key) const;
	/// https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	KeyEvent getKeyEvent();
	CharEvent getCharEvent();

	template<typename T>
	static void tryDiscardEvent(std::queue<T> &queue);

	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void endTick(std::shared_ptr<GameTimer> pGameTimer) override;
};


template<typename T>
void Keyboard::tryDiscardEvent(std::queue<T> &queue) {
	while (queue.size() > Keyboard::MaxQueueSize_)
		queue.pop();
}

}