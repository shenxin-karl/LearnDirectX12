#include <iostream>
#include <GameTimer/GameTimer.h>
#include "ShadowApp.h"

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	ShadowApp app;
	try {
		app.initialize();
		while (app.isRunning()) {
			pGameTimer->startNewFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
		app.destroy();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}
