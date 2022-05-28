#include "ImGuiProxy.h"
#include <Imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <Imgui/backends/imgui_impl_win32.h>
#include "Editor/Editor.h"
#include "Context/CommandQueue.h"
#include "Device/SwapChain.h"
#include "InputSystem/window.h"
#include "Texture/RenderTargetTexture.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void ShowExampleAppDockSpace(bool *p_open, bool &opt_padding, ImGuiDockNodeFlags &dockspace_flags) {
    // If you strip some features of, this demo is pretty much equivalent to calling DockSpaceOverViewport()!
    // In most cases you should be able to just call DockSpaceOverViewport() and ignore all the code below!
    // In this specific demo, we are not using DockSpaceOverViewport() because:
    // - we allow the host window to be floating/moveable instead of filling the viewport (when opt_fullscreen == false)
    // - we allow the host window to have padding (when opt_padding == true)
    // - we have a local menu bar in the host window (vs. you could use BeginMainMenuBar() + DockSpaceOverViewport() in your code!)
    // TL;DR; this demo is more complicated than what you would normally use.
    // If we removed all the options we are showcasing, this demo would become:
    //     void showExampleAppDockSpace()
    //     {
    //         ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
    //     }

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;


    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    // Disabling fullscreen would allow the window to be moved to the front of other windows,
	// which we can't undo at the moment without finer window depth/z control.
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Window Options")) {
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
            if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
            if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
            if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
            if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, true)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
            ImGui::Separator();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();
}


namespace ImGui {

void ImGuiProxy::initialize() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ED::Editor *pEditor = ED::Editor::instance();

    // create imgui srv descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(pEditor->getDevice()->getD3DDevice()->CreateDescriptorHeap(
        &desc,
        IID_PPV_ARGS(&_pDescriptorHeap)
    ));

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(pEditor->getInputSystem()->pWindow->getHWND());
    ImGui_ImplDX12_Init(pEditor->getDevice()->getD3DDevice(),
        dx12lib::kFrameResourceCount,
        pEditor->getDevice()->getDesc().backBufferFormat,
        _pDescriptorHeap.Get(),
        _pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        _pDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
    );

    pEditor->getInputSystem()->pWindow->setPrepareMessageCallBack(&ImGui_ImplWin32_WndProcHandler);
}

void ImGuiProxy::destroy() {
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiProxy::beginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // enable docking
	ShowExampleAppDockSpace(&_enableDockSpace, _optPadding, _dockSpaceFlag);
}

void ImGuiProxy::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
    auto pCmdQueue = ED::Editor::instance()->getDevice()->getCommandQueue();
    auto pDirectCtx = pCmdQueue->createDirectContextProxy();
    auto *pCmdList = pDirectCtx->getD3DCommandList();

    // Rendering
    ImGui::Render();

    auto pRenderTarget = ED::Editor::instance()->getSwapChain()->getRenderTarget2D();
    pDirectCtx->transitionBarrier(pRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
    auto RTV = pRenderTarget->getRTV();
    pCmdList->OMSetRenderTargets(1, RVPtr(RTV.getCPUDescriptorHandle()), FALSE, nullptr);
    pCmdList->SetDescriptorHeaps(1, _pDescriptorHeap.GetAddressOf());
    ImGui_ImplDX12_RenderDrawData(GetDrawData(), pCmdList);
    pDirectCtx->transitionBarrier(pRenderTarget, D3D12_RESOURCE_STATE_PRESENT);
    pDirectCtx->flushResourceBarriers();
    pCmdQueue->delayExecuted(pDirectCtx);
}

void ImGuiProxy::endTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    // Update and Render additional Platform Windows
    ED::Editor *pEditor = ED::Editor::instance();
    if (pEditor->isRunning() && GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, nullptr);
    }
}

}
