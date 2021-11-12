#include <iostream>
#include <Windows.h>
#include "GameTimer.h"

int main(int argc, char *argv[]) {
	com::GameTimer gt;
	for (size_t i = 0; i < 10; ++i) {
		std::cout << "totalTime: " << gt.totalTime() << std::endl;
		Sleep(100);
	}
	return 0;
}