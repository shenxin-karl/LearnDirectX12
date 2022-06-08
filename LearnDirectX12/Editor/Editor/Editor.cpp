#include "Editor.h"
#include <Imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <Imgui/backends/imgui_impl_win32.h>
#include <D3D/dx12libHelper/RenderTarget.h>
#include <Context/CommandQueue.h>
#include <InputSystem/window.h>
#include "Core/Utility/Utility.h"
#include "Editor/LogSystem/LogSystemEditor.h"
#include "Editor/MenuBar/EditorMenuBar.h"
#include "Scene/SceneManagerEditor.h"


namespace ED {

Editor::Editor() {
    _title = "Editor";
    _width = 1280;
    _height = 760;
    _canPause = false;
    pImGuiProxy = std::make_shared<ImGui::ImGuiProxy>();
    pMainMenuBar = std::make_shared<EditorMenuBar>();
    assert(pEditor == nullptr);
    pEditor = this;
}

Editor::~Editor() {

}

void Editor::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
    pImGuiProxy->initialize();
    pSceneMgr = std::make_shared<SceneManagerEditor>();
    _pInputSystem->pWindow->setCanPause(false);
    LogSystemEditor::emplace(std::make_shared<LogSystemEditor>("EditorLog.txt"));
    core::initDefaultSkyBox(pDirectCtx);
}

void Editor::onDestroy() {
    LogSystemEditor::destroy();
    pImGuiProxy->destroy();
}

void Editor::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    pImGuiProxy->beginTick(pGameTimer);
    static_cast<LogSystemEditor *>(core::LogSystem::instance())->updateCurrentTime();
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

        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            pMainMenuBar->drawMenuBar();
            ImGui::EndMainMenuBar();
        }

        // log
        static_cast<LogSystemEditor *>(core::LogSystem::instance())->drawLogWindow();

        pSceneMgr->drawSceneWindow();
        pSceneMgr->drawLightingWindow();
        pSceneMgr->drawHierarchyWindow();
        pSceneMgr->drawInspectorWindow();


        pImGuiProxy->tick(pGameTimer);
        renderTarget.unbind(pDirectCtx);
    }
    pCmdQueue->executeCommandList(pDirectCtx);
}

void Editor::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    pImGuiProxy->endTick(pGameTimer);

    auto pCmdQueue = _pDevice->getCommandQueue();
    pCmdQueue->signal(_pSwapChain);
}

Editor* &Editor::instance() noexcept {
    return pEditor;
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
