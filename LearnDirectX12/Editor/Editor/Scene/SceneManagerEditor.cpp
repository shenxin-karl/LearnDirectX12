#include <dx12lib/Context/ContextStd.h>
#include <dx12lib/Texture/TextureStd.h>
#include <D3D/dx12libHelper/RenderTarget.h>
#include "Core/Scene/SceneNode.h"
#include "Editor/Editor.h"
#include "Editor/Scene/SceneManagerEditor.h"
#include "Editor/ImGuiProxy/ImGuiProxy.h"
#include "Editor/ImGuiProxy/ImGuiInput.h"
#include "Editor/LogSystem/LogSystemEditor.h"
#include "Editor/MenuBar/EditorMenuBar.h"
#include "Editor/Scene/SceneNodeEditor.h"

namespace ED {

SceneManagerEditor::SceneManagerEditor() : _sceneWindowInputFilter("Scene") {
	_sceneWindowInputFilter.setImGuiInput(Editor::instance()->pImGuiProxy->getImGuiInput());

	auto pEditor = Editor::instance();
	auto *pMenu = pEditor->pMainMenuBar->registerBarItem("Windows");
	auto *pMenuItemGroup = pMenu->addSubItemGroup("SceneManagerEditor");
	pMenuItemGroup->menuItems.push_back([&]() {
		ImGui::MenuItem("Scene", nullptr, &showSceneWindow);
		ImGui::MenuItem("Hierarchy", nullptr, &showHierarchyWindow);
	});
}

void SceneManagerEditor::drawSceneWindow() {
	constexpr ImGuiWindowFlags kWindowFlags = ImGuiWindowFlags_NoScrollbar;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(_sceneWindowInputFilter.getWindowName().c_str(), &showSceneWindow, kWindowFlags);
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

	handleImGuiInput();
}

void SceneManagerEditor::drawHierarchyWindow() {
	if (!ImGui::Begin("Hierarchy", &showHierarchyWindow, 0)) {
		ImGui::End();
		return;
	}


	bool isSelected = false;
	bool isRightMouseButton = false;
	for (auto &pSceneNode : getNodeList()) {
		ImGui::Selectable(pSceneNode->getName().c_str());
		if (!isSelected) {
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				_pSelectedSceneNode = std::dynamic_pointer_cast<ISceneNodeEditor>(pSceneNode);
				isRightMouseButton = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
				isSelected = true;
				continue;
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				_pSelectedSceneNode = std::dynamic_pointer_cast<ISceneNodeEditor>(pSceneNode);
				isSelected = true;
			}
		}
	}

	if (isRightMouseButton && ImGui::BeginPopupContextWindow()) {
		if (ImGui::Selectable("111")) Log::debug("1111");
		ImGui::EndPopup();
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

void SceneManagerEditor::handleImGuiInput() {
	_sceneWindowInputFilter.update();

	auto pImGuiInput = _sceneWindowInputFilter.getImGuiInput();
	if (_sceneWindowInputFilter.isPassKeyBoardFilter()) {
		if (pImGuiInput->isKeyPressed(ImGuiKey_W))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::Forward);
		if (pImGuiInput->isKeyPressed(ImGuiKey_S))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::backward);
		if (pImGuiInput->isKeyPressed(ImGuiKey_A))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::Left);
		if (pImGuiInput->isKeyPressed(ImGuiKey_D))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::Right);
		if (pImGuiInput->isKeyPressed(ImGuiKey_Q))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::LeftRotate);
		if (pImGuiInput->isKeyPressed(ImGuiKey_E))
			_pMainCamera->setMotionState(d3d::FirstPersonCamera::RightRotate);
	}

	if (_sceneWindowInputFilter.isPassMouseFilter() && pImGuiInput->isMouseDown(ImGuiMouseButton_Right)) {
		ImVec2 mouseDelta = pImGuiInput->getMouseDelta();
		_pMainCamera->setYaw(_pMainCamera->getYaw() + mouseDelta.x);
		_pMainCamera->setPitch(_pMainCamera->getPitch() + mouseDelta.y);
	}
}

}
