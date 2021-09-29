#pragma once
#define NOMINMAX
#include <Windows.h>
#include <bitset>
#include <queue>
#include "CommonDefine.h"

class Window;
class Keyboard {
	friend class Window;
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
	Window	*window_;
	std::bitset<MaxKeyCodeSize_>	keyState_;
	std::bitset<MaxKeyCodeSize_>	characterState_;
	std::queue<KeyEvent>			keyQueue_;
	std::queue<CharEvent>			charQueue_;
public:
	Keyboard(Window *window);
	Keyboard(const Keyboard &) = delete;
	Keyboard &operator=(const Keyboard &) = delete;
	bool isKeyPressed(unsigned char key) const;
	bool isCharPressed(unsigned char key) const;
	KeyEvent readKey();
	CharEvent readChar();

	template<typename T>
	static void tryDiscardEvent(std::queue<T> &queue);
private:
	void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void tick();
};


template<typename T>
void Keyboard::tryDiscardEvent(std::queue<T> &queue) {
	while (queue.size() > Keyboard::MaxQueueSize_)
		queue.pop();
}