#pragma once
#define NOMINMAX
#include <windows.h>
#include <bitset>
#include <queue>
#include "CommonDefine.h"
#include "ITick.h"

class Keyboard : public ITick {
	struct CharEvent {
		enum State {
			Pressed,
			Invalid,
		};
		CharEvent() = default;
		CharEvent(State state, unsigned char character);
		unsigned char getCharacter() const;
		State getState() const;
		bool isPressed() const;
		bool isInvalid() const;
	private:
		State			state_;
		unsigned char	character_;
	};
	struct KeyEvent {
		enum State {
			Pressed,
			Released,
			Invalid,
		};
		KeyEvent() = default;
		KeyEvent(State state, unsigned char key);
		unsigned char getKey() const;
		State getState() const;
		bool isPressed() const;
		bool isReleased() const;
		bool isInvalid() const;
	private:
		State		  state_ = Invalid;
		unsigned char key_ = 0;
	};
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
	KeyEvent readKey();
	CharEvent readChar();

	template<typename T>
	static void tryDiscardEvent(std::queue<T> &queue);

	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual void beginTick() override;
	virtual void tick(GameTimer &dt) override;
	virtual void endTick() override;
};


template<typename T>
void Keyboard::tryDiscardEvent(std::queue<T> &queue) {
	while (queue.size() > Keyboard::MaxQueueSize_)
		queue.pop();
}