#pragma once
#include <GameTimer/GameTimer.h>

namespace core {

inline std::shared_ptr<com::GameTimer> pGlobalGameTimer = std::make_shared<com::GameTimer>();

inline std::shared_ptr<com::GameTimer> &getGlobalGameTimer() {
	return pGlobalGameTimer;
}

}