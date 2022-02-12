#include "BoxApp.h"
#include "GameTimer/GameTimer.h"
#include <iostream>
#include <memory>


int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	try {
		BoxApp app;
		app.initialize();
		while (!app.isRuning()) {
			pGameTimer->newFrame();
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
		app.destory();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}