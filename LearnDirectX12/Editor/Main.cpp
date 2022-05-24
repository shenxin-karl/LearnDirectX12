#include "Editor.h"
#include "GameTimer/GameTimer.h"
#include <iostream>
#include <memory>


int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	Editor editor;
	try {

		editor.initialize();
		while (editor.isRunning()) {
			pGameTimer->startNewFrame();
			editor.beginTick(pGameTimer);
			editor.tick(pGameTimer);
			editor.endTick(pGameTimer);
		}
		editor.destroy();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}
