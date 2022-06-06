#include "Editor/Editor.h"
#include "GameTimer/GameTimer.h"
#include <iostream>
#include <memory>
#include "Reflection/StaticReflection.hpp"
#include "Core/Utility/Utility.h"

int main() {
	std::shared_ptr<com::GameTimer> pGameTimer = core::getGlobalGameTimer();
	ED::Editor editor;
	try {
		editor.initialize();
		editor.setGameTimer(pGameTimer);
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
