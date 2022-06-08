#include "Core/Scene/SkyBoxNode.h"

namespace core {

SkyBoxNode::SkyBoxNode(const std::string &name) : SceneNode(name) {
}

std::shared_ptr<d3d::SkyBox> SkyBoxNode::getSkyBox() const {
	return _pSkyBox;
}

void SkyBoxNode::setSkyBox(std::shared_ptr<d3d::SkyBox> pSkyBox) {
	_pSkyBox = pSkyBox;
}

}
