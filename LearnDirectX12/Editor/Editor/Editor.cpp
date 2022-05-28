#include "Editor.h"
#include <Imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <Imgui/backends/imgui_impl_win32.h>
#include "D3D/dx12libHelper/RenderTarget.h"
#include "Context/CommandQueue.h"
#include "InputSystem/window.h"
#include "Scene/SceneManager.h"
#include "Log/EditorLog.h"

namespace ED {

Editor::Editor() {
    _title = "Editor";
    _width = 1280;
    _height = 760;
    _canPause = false;
    _pImGuiProxy = std::make_shared<ImGui::ImGuiProxy>();
    assert(_pEditor == nullptr);
    _pEditor = this;
}

Editor::~Editor() {

}

void Editor::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
    _pImGuiProxy->initialize();
    _pSceneMgr = std::make_shared<SceneManager>();

    auto pHierarchyWindow = std::make_shared<HierarchyWindow>(_pSceneMgr);
    auto pInspectorWindow = std::make_shared<InspectorWindow>(_pSceneMgr);

    attachWindow("Hierarchy", pHierarchyWindow);
    attachWindow("Inspector", pInspectorWindow);
    attachWindow("Log", EditorLog::instance());
}

void Editor::onDestroy() {
    _pImGuiProxy->destroy();
}

void Editor::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    _pImGuiProxy->beginTick(pGameTimer);
    EditorLog::instance()->updateCurrentTime();
}

void Editor::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    auto pCmdQueue = _pDevice->getCommandQueue();
    auto pDirectCtx = pCmdQueue->createDirectContextProxy();
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        static float4 clearColor{ 0.f };
        d3d::RenderTarget renderTarget(_pSwapChain);
        renderTarget.bind(pDirectCtx);
        renderTarget.clear(pDirectCtx, clearColor);

        showEditorMainMenuBar();

        for (auto &&[name, ptr] : _windows) {
			if (*ptr->getOpenFlagPtr())
                ptr->showWindow();
        }

        _pImGuiProxy->tick(pGameTimer);
        renderTarget.unbind(pDirectCtx);
    }
    pCmdQueue->executeCommandList(pDirectCtx);
}

void Editor::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    _pImGuiProxy->endTick(pGameTimer);

    auto pCmdQueue = _pDevice->getCommandQueue();
    pCmdQueue->signal(_pSwapChain);
}

void Editor::attachWindow(const std::string &name, std::shared_ptr<IEditorWindow> pWindow) {
    _windows[name] = pWindow;
}

void Editor::detachWindow(const std::string &name) {
    _windows.erase(name);
}

void Editor::attachMenu(const std::string &name, std::shared_ptr<IEditorMenu> pMenu) {
    _menus[name] = pMenu;
}

void Editor::detachMenu(const std::string &name) {
    _menus.erase(name);
}

void Editor::showEditorMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Windows")) {
            for (auto &&[name, ptr] : _windows) 
	            ImGui::MenuItem(name.c_str(), nullptr, ptr->getOpenFlagPtr());
			ImGui::EndMenu();    
        }
        for (auto &&[name, ptr] : _menus) {
	        if (ImGui::BeginMenu(name.c_str())) {
				ptr->showMenu();
                ImGui::EndMenu();
            }
        }
		ImGui::EndMainMenuBar();
    }
}

Editor *Editor::instance() noexcept {
    return _pEditor;
}

std::shared_ptr<com::InputSystem> Editor::getInputSystem() const {
    return _pInputSystem;
}

std::shared_ptr<dx12lib::Device> Editor::getDevice() const {
    return _pDevice;
}

std::shared_ptr<dx12lib::SwapChain> Editor::getSwapChain() const {
    return _pSwapChain;
}

}
