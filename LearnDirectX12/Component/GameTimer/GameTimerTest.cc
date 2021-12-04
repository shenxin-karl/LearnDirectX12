#include <iostream>
#include <Windows.h>
#include "GameTimer.h"

int main(int argc, char *argv[]) {
	com::GameTimer gt;
	for (size_t i = 0; i < 10; ++i) {
		std::cout << "totalTime: " << gt.getTotalTime() << std::endl;
		Sleep(100);
	}

	int second = 0;
	while (second < 10) {
		gt.newFrame();
		if (gt.oneSecondTrigger()) {
			std::cout << "fps: " << gt.FPS() << std::endl;
			std::cout << "mspf: " << gt.mspf() << std::endl;
			++second;
		}
	}
	return 0;
}