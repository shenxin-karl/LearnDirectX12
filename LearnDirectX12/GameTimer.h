#pragma once
#include <chrono>

namespace chrono = std::chrono;
class GameTimer {
	GameTimer();
	void reset();
	void start();
	void stop();
	void tick();
	float totalTime() const;
	float deltaTime() const;
private:
	chrono::steady_clock::time_point baseTime_;
	chrono::steady_clock::time_point prevTime_;
	chrono::steady_clock::time_point stoppedTime_;
	float							 deltaTime_;
	float							 pausedTime_;
	bool							 stopped_;
};

