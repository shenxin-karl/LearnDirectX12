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
}

void ImGuiProxy::tick(std::shared_ptr<com::GameTimer> pGameTimer) {
    auto pCmdQueue = ED::Editor::instance()->getDevice()->getCommandQueue();
    auto pDirectCtx = pCmdQueue->createDirectContextProxy();
    auto *pCmdList = pDirectCtx->getD3DCommandList();

    // Rendering
    ImGui::Render();
    auto RTV = ED::Editor::instance()->getSwapChain()->getRenderTarget2D()->getRTV();
    pCmdList->OMSetRenderTargets(1, RVPtr(RTV.getCPUDescriptorHandle()), FALSE, nullptr);
    pCmdList->SetDescriptorHeaps(1, _pDescriptorHeap.GetAddressOf());
    ImGui_ImplDX12_RenderDrawData(GetDrawData(), pCmdList);
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
