#pragma once
#include "BaseApp/BaseApp.h"
#include "Math/MathStd.hpp"
#include "ImGuiProxy/ImGuiProxy.h"

using namespace Math;

class Editor : public com::BaseApp {
public:
	Editor();
	~Editor() override = default;
protected:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
public:
	static inline Editor *_pEditor = nullptr;
	static Editor *instance() noexcept;
	std::shared_ptr<com::InputSystem> getInputSystem() const;
	std::shared_ptr<dx12lib::Device> getDevice() const;
	std::shared_ptr<dx12lib::SwapChain> getSwapChain() const;
private:
	std::unique_ptr<ImGui::ImGuiProxy> _pImGuiProxy;
};
