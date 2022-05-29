#include "Scene/InspectorWindow.h"
#include "Imgui/imgui.h"

namespace ED {

InspectorWindow::InspectorWindow(std::shared_ptr<SceneManager> pSceneMgr) : _pSceneMgr(pSceneMgr) {
}

void InspectorWindow::showWindow() {
	if (ImGui::Begin("Inspector", &_openInspector, 0)) {
		
	}
	ImGui::End();
}

bool *InspectorWindow::getOpenFlagPtr() {
	return &_openInspector;
}

}
