#include <cassert>
#include "Scene/SceneManager.h"
#include "SceneNode.h"
#include "Context/CommandQueue.h"
#include "Editor/Editor.h"
#include "Imgui/imgui.h"
#include <DirectXColors.h>

#include "GameTimer/GameTimer.h"

namespace ED {

bool SceneManager::addNode(std::shared_ptr<SceneNode> pSceneNode) {
	auto iter = _nodeMap.find(pSceneNode->getName());
	if (iter != _nodeMap.end()) {
		assert(false);
		return false;
	}

	auto nodeIter = _nodeList.insert(_nodeList.end(), pSceneNode);
	_nodeMap[pSceneNode->getName()] = nodeIter;
	return true;
}

void SceneManager::eraseNode(const std::string &name) {
	auto iter = _nodeMap.find(name);
	if (iter == _nodeMap.end())
		return;

	auto nodeMapIter = _nodeMap.find(name);
	auto nodeIter = nodeMapIter->second;
	_nodeList.erase(nodeIter);
	_nodeMap.erase(nodeMapIter);
}

size_t SceneManager::getNodeSize() const {
	return _nodeList.size();
}

std::list<std::shared_ptr<SceneNode>> & SceneManager::getNodeList() {
	return _nodeList;
}

void SceneManager::renderScene(d3d::RenderTarget renderTarget, dx12lib::DirectContextProxy pDirectCtx) {
	auto *pEditor = Editor::instance();
	auto pGameTimer = pEditor->getGameTimer();
	renderTarget.bind(pDirectCtx);
	{
		float totalTime = pGameTimer->getTotalTime();
		float4 color = {
			std::cos(totalTime) * 0.5f + 0.5f,
			std::sin(totalTime) * 0.5f + 0.5f,
			1.f,
			1.f,
		};
		renderTarget.clear(pDirectCtx, color);
	}
	renderTarget.unbind(pDirectCtx);
}

}
