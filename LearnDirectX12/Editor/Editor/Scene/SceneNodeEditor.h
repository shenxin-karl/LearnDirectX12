#pragma once
#include "Core/Scene/SceneNode.h"

namespace ED {

class ISceneNodeEditor : virtual public core::SceneNode {
public:
	using core::SceneNode::SceneNode;
	virtual void showInspector() = 0;
};

}
