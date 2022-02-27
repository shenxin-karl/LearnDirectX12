#include "LandAndWater.h"
#include "GameTimer/GameTimer.h"
#include <iostream>


int main() {

	auto pGameTimer = std::make_shared<com::GameTimer>();
	LandAndWater app;
	try {
		app.initialize();
		while (app.isRuning()) {
			pGameTimer->newFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
		app.destory();
	} catch(const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}