#pragma once
#include <dx12lib/dx12libStd.h>
#include <BaseApp/BaseApp.h>


class ShadowApp : com::BaseApp {
public:
	ShadowApp();
	~ShadowApp() override = default;
private:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onResize(dx12lib::DirectContextProxy pDirectCtx, int width, int height) override;
};