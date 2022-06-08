#pragma once
#include <dx12lib/Context/ContextProxy.hpp>
#include <GameTimer/GameTimer.h>

namespace d3d {
	class SkyBox;
};

namespace core {

inline std::shared_ptr<com::GameTimer> pGlobalGameTimer = std::make_shared<com::GameTimer>();
inline std::shared_ptr<com::GameTimer> &getGlobalGameTimer() {
	return pGlobalGameTimer;
}

void initDefaultSkyBox(dx12lib::GraphicsContextProxy pGraphicsCtx);
inline std::shared_ptr<d3d::SkyBox> pDefaultSkyBox = nullptr;
inline std::shared_ptr<d3d::SkyBox> getDefaultSkyBox() {
	return pDefaultSkyBox;
}

}