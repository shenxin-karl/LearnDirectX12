#include "Core/Scene/SceneNode.h"

namespace core {

SceneNode::SceneNode(const std::string &name) : _name(name) {
}

const std::string &SceneNode::getName() const {
	return _name;
}

}
