#pragma once
#include <memory>
#include <unordered_map>
#include "Editor/IEditorItem.h"

namespace ED {

class SceneNode;

class SceneManager : public IEditorWindow {
public:
	SceneManager() = default;
	SceneManager(const SceneManager &) = delete;
	void showWindow() override;
	bool addNode(std::shared_ptr<SceneNode> pSceneNode);
	void eraseNode(const std::string &name);
	size_t getNodeSize() const;
private:
	using NodeIndexMap = std::unordered_map<std::string, std::list<std::shared_ptr<SceneNode>>::iterator>;
private:
	bool _openHierarchy = true;
	bool _openInspector = true;
	NodeIndexMap _nodeMap;
	std::list<std::shared_ptr<SceneNode>> _nodes;
};

}
