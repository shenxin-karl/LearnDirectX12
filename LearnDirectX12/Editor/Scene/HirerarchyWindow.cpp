#include "Scene/HirerarchyWindow.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "Imgui/imgui.h"

namespace ED {

HierarchyWindow::HierarchyWindow(std::shared_ptr<SceneManager> pSceneMgr) : _pSceneMgr(pSceneMgr) {
}

void HierarchyWindow::showWindow() {
	if (ImGui::Begin("Hierarchy", &_openHierarchy, 0)) {
		if (ImGui::TreeNode("SceneNodes")) {
			for (auto &pSceneNode : _pSceneMgr->getNodeList()) {
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

bool *HierarchyWindow::getOpenFlagPtr() {
	return &_openHierarchy;
}

}
