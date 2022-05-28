#include "Scene/SceneManager.h"
#include "SceneNode.h"
#include <cassert>

#include "Editor/Editor.h"
#include "Imgui/imgui.h"

namespace ED {

bool SceneManager::addNode(std::shared_ptr<SceneNode> pSceneNode) {
	auto iter = _nodeMap.find(pSceneNode->getName());
	if (iter != _nodeMap.end()) {
		assert(false);
		return false;
	}

	auto nodeIter = _nodes.insert(_nodes.end(), pSceneNode);
	_nodeMap[pSceneNode->getName()] = nodeIter;
	return true;
}

void SceneManager::eraseNode(const std::string &name) {
	auto iter = _nodeMap.find(name);
	if (iter == _nodeMap.end())
		return;

	auto nodeMapIter = _nodeMap.find(name);
	auto nodeIter = nodeMapIter->second;
	_nodes.erase(nodeIter);
	_nodeMap.erase(nodeMapIter);
}

size_t SceneManager::getNodeSize() const {
	return _nodes.size();
}

std::list<std::shared_ptr<SceneNode>> & SceneManager::getNodeList() {
	return _nodes;
}

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
