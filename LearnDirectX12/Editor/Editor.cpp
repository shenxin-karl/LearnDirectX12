#include "Editor.h"
#include <Imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <Imgui/backends/imgui_impl_win32.h>
#include "D3D/dx12libHelper/RenderTarget.h"
#include "Context/CommandQueue.h"
#include "InputSystem/window.h"

Editor::Editor() {
    _pImGuiProxy = std::make_unique<ImGui::ImGuiProxy>();
    assert(_pEditor == nullptr);
    _pEditor = this;
}

void Editor::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
    _pImGuiProxy->initialize();
}

void Editor::onDestroy() {
    _pImGuiProxy->destroy();
}

void Editor::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    _pImGuiProxy->beginTick(pGameTimer);
}

void Editor::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    auto pCmdQueue = _pDevice->getCommandQueue();
    auto pDirectCtx = pCmdQueue->createDirectContextProxy();
    {
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        static bool showDemoWindow = true;
        static bool showAnotherWindow = false;
        static float4 clearColor{ 0.f };

        d3d::RenderTarget renderTarget(_pSwapChain);
        renderTarget.bind(pDirectCtx);
        renderTarget.clear(pDirectCtx, clearColor);


        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &showAnotherWindow);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clearColor)); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (showAnotherWindow)
        {
            ImGui::Begin("Another Window", &showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                showAnotherWindow = false;
            ImGui::End();
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
