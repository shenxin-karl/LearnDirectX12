#include "Scene/SceneNode.h"

namespace ED {

SceneNode::SceneNode(const std::string &name) : _name(name) {
}

const std::string &SceneNode::getName() const {
	return _name;
}

}
