#pragma once
#include "BaseApp/BaseApp.h"
#include "D3D/Camera.h"

class BasicTessellationApp : public com::BaseApp {
public:
	BasicTessellationApp();
	~BasicTessellationApp() override;
protected:
	void onInitialize(dx12lib::DirectContextProxy pCmdList) override;
	void onDistory() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pCmdList, int width, int height) override;
private:
	std::unique_ptr<d3d::CoronaCamera> _pCamera;
};