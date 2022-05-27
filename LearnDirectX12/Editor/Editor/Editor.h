#pragma once
#include "BaseApp/BaseApp.h"
#include "Math/MathStd.hpp"
#include "ImGuiProxy/ImGuiProxy.h"
#include "IEditorItem.h"

using namespace Math;

namespace ED {

class SceneManager;

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
	void attachWindow(const std::string &name, std::shared_ptr<IEditorWindow> pWindow);
	void detachWindow(const std::string &name);
public:
	static inline Editor *_pEditor = nullptr;
	static Editor *instance() noexcept;
	std::shared_ptr<com::InputSystem> getInputSystem() const;
	std::shared_ptr<dx12lib::Device> getDevice() const;
	std::shared_ptr<dx12lib::SwapChain> getSwapChain() const;
private:
	std::unique_ptr<SceneManager>	   _pSceneMgr;
	std::unique_ptr<ImGui::ImGuiProxy> _pImGuiProxy;
	std::unordered_map<std::string, std::shared_ptr<IEditorWindow>> _windows;
};

}