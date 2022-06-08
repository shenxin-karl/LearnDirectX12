#pragma once
#include <D3D/Sky/SkyBox.h>
#include "Core/Scene/SceneNode.h"

namespace core {

class SkyBoxNode : public SceneNode {
public:
	explicit SkyBoxNode(const std::string &name);
	std::shared_ptr<d3d::SkyBox> getSkyBox() const;
	void setSkyBox(std::shared_ptr<d3d::SkyBox> pSkyBox);
protected:
	std::shared_ptr<d3d::SkyBox> _pSkyBox;
};

}
