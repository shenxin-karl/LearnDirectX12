#include "BoxApp.h"
#include "GameTimer/GameTimer.h"
#include <iostream>
#include <memory>

void changeWorkDirectory() {
	char buffer[128];
	GetCurrentDirectory(std::size(buffer), buffer);
	std::string path = buffer;
	std::string substr = "Build\\";
	if (auto pos = path.find(substr); pos != std::string::npos) {
		path.replace(pos, substr.length(), "");
		SetCurrentDirectory(path.c_str());
	}
}

int main() {
	changeWorkDirectory();
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	BoxApp app;
	try {
		app.initialize();
		while (!app.shouldClose()) {
			app.beginTick(pGameTimer);
			app.tick(pGameTimer);
			app.endTick(pGameTimer);
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		OutputDebugStringA(e.what());
	}
	return 0;
}