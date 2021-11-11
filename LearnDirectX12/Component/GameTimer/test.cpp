#include <iostream>
#include <format>
#include <Windows.h>
#include "GameTimer.h"

int main() {
	GameTimer timer;
	std::cout << timer.totalTime() << std::endl;
	for (int i = 0; i < 10; ++i) {
		timer.tick();
		std::cout << std::format("timer.totalTime: {}, timer.deltaTime: {}\n",
			timer.totalTime(),
			timer.deltaTime()
		);

		if (i % 2 == 0)
			timer.stop();
		else
			timer.start();

		Sleep(1000);
	}
	std::cout << std::format("timer.totalTime: {}, timer.deltaTime: {}\n",
		timer.totalTime(),
		timer.deltaTime()
	);
}