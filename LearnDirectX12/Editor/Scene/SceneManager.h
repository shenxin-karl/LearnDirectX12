#pragma once
#include <ITick.h>
#include <memory>
#include <unordered_map>
#include "Editor/IEditorItem.h"

namespace ED {

class SceneNode;

class SceneManager {
public:
	SceneManager() = default;
	SceneManager(const SceneManager &) = delete;
	bool addNode(std::shared_ptr<SceneNode> pSceneNode);
	void eraseNode(const std::string &name);
	size_t getNodeSize() const;
	std::list<std::shared_ptr<SceneNode>> &getNodeList();
private:
	using NodeIndexMap = std::unordered_map<std::string, std::list<std::shared_ptr<SceneNode>>::iterator>;
private:
	bool _openHierarchy = true;
	bool _openInspector = true;
	NodeIndexMap _nodeMap;
	std::list<std::shared_ptr<SceneNode>> _nodes;
};

class HierarchyWindow : public IEditorWindow {
public:
	HierarchyWindow(std::shared_ptr<SceneManager> pSceneMgr);
	void showWindow() override;
	bool *getOpenFlagPtr() override;
private:
	bool _openHierarchy = true;
	std::shared_ptr<SceneManager> _pSceneMgr;
};

class InspectorWindow : public IEditorWindow {
public:
	InspectorWindow(std::shared_ptr<SceneManager> pSceneMgr);
	void showWindow() override;
	bool *getOpenFlagPtr() override;
private:
	bool _openInspector;
	std::shared_ptr<SceneManager> _pSceneMgr;
};

}
