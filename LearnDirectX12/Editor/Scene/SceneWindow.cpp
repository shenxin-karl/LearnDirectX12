#include "SceneWindow.h"
#include "Context/CommandQueue.h"
#include <dx12lib/Texture/TextureStd.h>

#include "SceneManager.h"
#include "Device/SwapChain.h"
#include "Editor/Editor.h"
#include "Imgui/imgui.h"

namespace ED {

void SceneWindow::showWindow() {
	constexpr ImGuiWindowFlags kWindowFlags = ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Scene", &_show, kWindowFlags);
	{
		Editor *pEditor = Editor::instance();
		auto pSharedDevice = pEditor->getDevice();
		auto pCmdQueue = pSharedDevice->getCommandQueue();
		auto pDirectCtx = pCmdQueue->createDirectContextProxy();
		auto pSwapChain = pEditor->getSwapChain();

		size_t width = static_cast<size_t>(ImGui::GetWindowWidth());
		size_t height = static_cast<size_t>(ImGui::GetWindowHeight());
		if (_width != width || _height != height)
			resizeRenderTarget(width, height, pDirectCtx);

		d3d::RenderTarget renderTarget(_pRenderTarget2D, _pDepthStencil2D);
		renderTarget.setFinalState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		pEditor->_pSceneMgr->renderScene(std::move(renderTarget), pDirectCtx);
		pEditor->_pImGuiProxy->image(ImGui::ImGuiProxy::Scene, _pRenderTarget2D->getSRV(), _width, _height);
		pCmdQueue->delayExecuted(pDirectCtx);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

bool *SceneWindow::getOpenFlagPtr() {
	return &_show;
}

void SceneWindow::resizeRenderTarget(size_t width, size_t height, dx12lib::DirectContextProxy pDirectCtx) {
	_width = width;
	_height = height;
	pDirectCtx->trackResource(std::move(_pRenderTarget2D));
	pDirectCtx->trackResource(std::move(_pDepthStencil2D));
	_pRenderTarget2D = pDirectCtx->createRenderTarget2D(width, height);
	_pDepthStencil2D = pDirectCtx->createDepthStencil2D(width, height);
}

}
