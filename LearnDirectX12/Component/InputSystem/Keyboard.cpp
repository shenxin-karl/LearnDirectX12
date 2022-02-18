#include "Keyboard.h"

namespace com {

Keyboard::Keyboard() {
}

bool Keyboard::isKeyPressed(unsigned char key) const {
	return keyState_.test(key);
}

bool Keyboard::isCharPressed(unsigned char key) const {
	return characterState_.test(key);
}

KeyEvent Keyboard::readKey() {
	if (keycodeQueue_.empty())
		return KeyEvent{};

	auto res = keycodeQueue_.front();
	keycodeQueue_.pop();
	return res;
}

CharEvent Keyboard::readChar() {
	if (characterQueue_.empty())
		return CharEvent{};
	auto res = characterQueue_.front();
	characterQueue_.pop();
	return res;
}


void Keyboard::handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_KEYDOWN:
		keyState_.set(wParam);
		keycodeQueue_.emplace(KeyState::Pressed, static_cast<char>(wParam));
		break;
	case WM_KEYUP:
		keyState_.set(wParam, false);
		keycodeQueue_.emplace(KeyState::Released, static_cast<char>(wParam));
		break;
	case WM_CHAR:
		characterState_.set(wParam);
		characterQueue_.emplace(KeyState::Pressed, static_cast<char>(wParam));
		break;
	}
}

void Keyboard::endTick(std::shared_ptr<GameTimer> pGameTimer) {
	tryDiscardEvent(keycodeQueue_);
	tryDiscardEvent(characterQueue_);
}

unsigned char KeyEvent::getKey() const {
	return key_;
}

KeyState KeyEvent::getState() const {
	return state_;
}

bool KeyEvent::isPressed() const {
	return state_ == KeyState::Pressed;
}

bool KeyEvent::isReleased() const {
	return state_ == KeyState::Released;
}

bool KeyEvent::isInvalid() const {
	return state_ == KeyState::Invalid;
}

KeyEvent::operator bool() const {
	return !isInvalid();
}

KeyEvent::KeyEvent(KeyState state, unsigned char key) : state_(state), key_(key) {
}

unsigned char CharEvent::getCharacter() const {
	return character_;
}

KeyState CharEvent::getState() const {
	return state_;
}

bool CharEvent::isPressed() const {
	return state_ == KeyState::Pressed;
}

bool CharEvent::isInvalid() const {
	return state_ == KeyState::Invalid;
}

CharEvent::operator bool() const {
	return state_ != KeyState::Invalid;
}

CharEvent::CharEvent(KeyState state, unsigned char character)
	: state_(state), character_(character) {
}

}