#pragma once
#include <Singleton/Singleton.hpp>
#include <BaseApp/BaseApp.h>
#include <Math/MathStd.hpp>
#include "Editor/ImGuiProxy/ImGuiProxy.h"

using namespace Math;

namespace ED {

class SceneManagerEditor;
class EditorMenuBar;

class Editor : public com::BaseApp {
public:
	Editor();
	~Editor() override;
protected:
	void onInitialize(dx12lib::DirectContextProxy pDirectCtx) override;
	void onDestroy() override;
	void onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
	void onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) override;
public:
	static inline Editor *pEditor = nullptr;
	static Editor* &instance() noexcept;

	std::shared_ptr<com::InputSystem> getInputSystem() const;
	std::shared_ptr<dx12lib::Device> getDevice() const;
	std::shared_ptr<dx12lib::SwapChain> getSwapChain() const;

	std::shared_ptr<EditorMenuBar>	     pMainMenuBar;
	std::shared_ptr<ImGui::ImGuiProxy>   pImGuiProxy;
	std::shared_ptr<SceneManagerEditor>	 pSceneMgr;
};

}