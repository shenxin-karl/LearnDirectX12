#include "Keyboard.h"

Keyboard::Keyboard(Window *window) : window_(window) {
}

bool Keyboard::isKeyPressed(unsigned char key) const {
	return keyState_.test(key);
}

bool Keyboard::isCharPressed(unsigned char key) const {
	return characterState_.test(key);
}

Keyboard::KeyEvent Keyboard::readKey() {
	if (keyQueue_.empty())
		return KeyEvent{};

	auto res = keyQueue_.front();
	keyQueue_.pop();
	return res;
}

Keyboard::CharEvent Keyboard::readChar() {
	if (charQueue_.empty())
		return CharEvent{};
	auto res = charQueue_.front();
	charQueue_.pop();
	return res;
}


void Keyboard::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_KEYDOWN:
		keyState_.set(wParam);
		keyQueue_.emplace(KeyEvent::Pressed, static_cast<char>(wParam));
		break;
	case WM_KEYUP:
		keyState_.set(wParam, false);
		keyQueue_.emplace(KeyEvent::Released, static_cast<char>(wParam));
		break;
	case WM_CHAR:
		characterState_.set(wParam);
		charQueue_.emplace(CharEvent::Pressed, static_cast<char>(wParam));
		break;
	}
}

void Keyboard::tick() {
	characterState_.reset();
	tryDiscardEvent(keyQueue_);
	tryDiscardEvent(charQueue_);
}

unsigned char Keyboard::KeyEvent::getKey() const {
	return key_;
}

Keyboard::KeyEvent::State Keyboard::KeyEvent::getState() const {
	return state_;
}

bool Keyboard::KeyEvent::isPressed() const {
	return state_ == Pressed;
}

bool Keyboard::KeyEvent::isReleased() const {
	return state_ == Released;
}

bool Keyboard::KeyEvent::isInvalid() const {
	return state_ == Invalid;
}

Keyboard::KeyEvent::KeyEvent(State state, unsigned char key) : state_(state), key_(key) {
}

unsigned char Keyboard::CharEvent::getCharacter() const {
	return character_;
}

Keyboard::CharEvent::State Keyboard::CharEvent::getState() const {
	return state_;
}

bool Keyboard::CharEvent::isPressed() const {
	return state_ == Pressed;
}

bool Keyboard::CharEvent::isInvalid() const {
	return state_ == Invalid;
}

Keyboard::CharEvent::CharEvent(State state, unsigned char character) 
: state_(state), character_(character) {
}
