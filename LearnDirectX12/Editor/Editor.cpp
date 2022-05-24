#include "Editor.h"
#include <Imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx12.h>
#include <Imgui/backends/imgui_impl_win32.h>
#include "D3D/dx12libHelper/RenderTarget.h"
#include "Context/CommandQueue.h"
#include "InputSystem/window.h"

void Editor::onInitialize(dx12lib::DirectContextProxy pDirectCtx) {
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

    // create imgui srv descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(_pDevice->getD3DDevice()->CreateDescriptorHeap(
		&desc,
        IID_PPV_ARGS(&_pImGuiDescriptorHeap)
    ));

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(_pInputSystem->pWindow->getHWND());
    ImGui_ImplDX12_Init(_pDevice->getD3DDevice(), 
        dx12lib::kFrameResourceCount, 
        _pDevice->getDesc().backBufferFormat, 
        _pImGuiDescriptorHeap.Get(),
        _pImGuiDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        _pImGuiDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
    );
}

void Editor::onDestroy() {
    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Editor::onBeginTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Editor::onTick(std::shared_ptr<com::GameTimer> pGameTimer) {
    auto pCmdQueue = _pDevice->getCommandQueue();
    auto pDirectCtx = pCmdQueue->createDirectContextProxy();
    {
		d3d::RenderTarget renderTarget(_pSwapChain);
	    renderTarget.bind(pDirectCtx);
        renderTarget.unbind(pDirectCtx);
    }

    pCmdQueue->executeCommandList(pDirectCtx);
    pCmdQueue->signal(_pSwapChain);
}

void Editor::onEndTick(std::shared_ptr<com::GameTimer> pGameTimer) {
	
}
