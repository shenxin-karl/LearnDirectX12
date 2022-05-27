#include "Scene/SceneManager.h"
#include "SceneNode.h"
#include <cassert>

#include "Imgui/imgui.h"

namespace ED {

void SceneManager::showWindow() {

	if (!ImGui::Begin("Hierarchy", &_openHierarchy, 0))
	{
		if (ImGui::TreeNode("SceneNode")) {
			for (auto &pSceneNode : _nodes)

			ImGui::TreePop();
		}


		ImGui::End();
	}

}

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

}
