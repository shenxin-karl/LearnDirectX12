#include <dx12lib/Context/ContextStd.h>
#include <dx12lib/Texture/TextureStd.h>
#include <D3D/dx12libHelper/RenderTarget.h>
#include "Core/Scene/SceneNode.h"
#include "Editor/Scene/SceneManagerEditor.h"
#include "Editor/ImGuiProxy/ImGuiProxy.h"
#include "Editor/Editor.h"

namespace ED {

void SceneManagerEditor::drawSceneWindow() {
	constexpr ImGuiWindowFlags kWindowFlags = ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Scene", &showSceneWindow, kWindowFlags);
	{
		Editor *pEditor = Editor::instance();
		auto pSharedDevice = pEditor->getDevice();
		auto pCmdQueue = pSharedDevice->getCommandQueue();
		auto pDirectCtx = pCmdQueue->createDirectContextProxy();
		auto pSwapChain = pEditor->getSwapChain();

		size_t width = static_cast<size_t>(ImGui::GetWindowWidth());
		size_t height = static_cast<size_t>(ImGui::GetWindowHeight());
		if (_sceneWidth != width || _sceneHeight != height)
			resizeRenderTarget(width, height, pDirectCtx);

		d3d::RenderTarget renderTarget(_pRenderTarget2D, _pDepthStencil2D);
		renderTarget.setFinalState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		SceneManager::renderScene(std::move(renderTarget), pDirectCtx);

		pEditor->pImGuiProxy->image(ImGui::ImGuiProxy::Scene, _pRenderTarget2D->getSRV(), width, height);
		pCmdQueue->delayExecuted(pDirectCtx);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void SceneManagerEditor::drawHierarchyWindow() {
	if (ImGui::Begin("Hierarchy", &showHierarchyWindow, 0)) {
		if (ImGui::TreeNode("SceneNodes")) {
			for (auto &pSceneNode : getNodeList()) {
				ImGui::Selectable(pSceneNode->getName().c_str());
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::Button("Delete"))
						ImGui::CloseCurrentPopup();
				}
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void SceneManagerEditor::drawInspectorWindow() {
	if (ImGui::Begin("Inspector", &showInspectorWindow, 0)) {

	}
	ImGui::End();
}

void SceneManagerEditor::resizeRenderTarget(size_t width, size_t height, dx12lib::GraphicsContextProxy pGraphicsCtx) {
	_sceneWidth = width;
	_sceneHeight = height;
	pGraphicsCtx->trackResource(std::move(_pRenderTarget2D));
	pGraphicsCtx->trackResource(std::move(_pDepthStencil2D));
	_pRenderTarget2D = pGraphicsCtx->createRenderTarget2D(width, height);
	_pDepthStencil2D = pGraphicsCtx->createDepthStencil2D(width, height);
}

}
