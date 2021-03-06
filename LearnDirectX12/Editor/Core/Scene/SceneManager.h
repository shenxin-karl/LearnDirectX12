#pragma once
#include <memory>
#include <unordered_map>
#include <ITick.h>
#include <D3D/dx12libHelper/RenderTarget.h>
#include <D3D/Tool/Camera.h>

namespace d3d {
	class RenderTarget;
}

namespace core {

class SceneNode;

class SceneManager {
public:
	SceneManager() = default;
	SceneManager(const SceneManager &) = delete;
	bool addNode(std::shared_ptr<SceneNode> pSceneNode);
	void eraseNode(const std::string &name);
	size_t getNodeSize() const;
	std::list<std::shared_ptr<SceneNode>> &getNodeList();
	void renderScene(d3d::RenderTarget renderTarget, dx12lib::DirectContextProxy pDirectCtx);
protected:
	using NodeIndexMap = std::unordered_map<std::string, std::list<std::shared_ptr<SceneNode>>::iterator>;
	using NodeList = std::list<std::shared_ptr<SceneNode>>;
	NodeList _nodeList;
	NodeIndexMap _nodeMap;

	std::string	_skyBoxName;
	std::shared_ptr<d3d::SkyBox>		    _pSkyBox;

	std::unique_ptr<d3d::FirstPersonCamera> _pMainCamera;
};

}
