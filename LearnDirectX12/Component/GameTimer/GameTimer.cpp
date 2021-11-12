#include "GameTimer.h"

namespace com {

GameTimer::GameTimer() {
	reset();
}

void GameTimer::reset() {
	baseTime_ = chrono::steady_clock::now();
	prevTime_ = baseTime_;
	stopped_ = false;
	deltaTime_ = 0.f;
	pausedTime_ = 0.f;
}

void GameTimer::start() {
	if (!stopped_)
		return;

	stopped_ = false;
	auto currTime = chrono::steady_clock::now();
	prevTime_ = currTime;
	chrono::duration<float> diff = currTime - stoppedTime_;
	pausedTime_ += diff.count();
}

void GameTimer::stop() {
	if (stopped_)
		return;

	deltaTime_ = 0.f;
	stopped_ = true;
	stoppedTime_ = chrono::steady_clock::now();
}

void GameTimer::tick() {
	if (stopped_)
		return;

	auto currTime = chrono::steady_clock::now();
	chrono::duration<float> diff = currTime - prevTime_;
	prevTime_ = currTime;
	deltaTime_ = diff.count();
}

float GameTimer::totalTime() const {
	chrono::duration<float> diff = chrono::steady_clock::now() - baseTime_;
	return diff.count() - pausedTime_;
}

float GameTimer::deltaTime() const {
	return deltaTime_;
}

}