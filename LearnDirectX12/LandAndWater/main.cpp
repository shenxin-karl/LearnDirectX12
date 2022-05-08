#include "LandAndWater.h"
#include "GameTimer/GameTimer.h"
#include <iostream>


int main() {

	auto pGameTimer = std::make_shared<com::GameTimer>();
	LandAndWater app;
	try {
		app.initialize();
		while (app.isRunning()) {
			pGameTimer->startNewFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
		app.destroy();
	} catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}