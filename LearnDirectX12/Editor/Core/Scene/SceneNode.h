#pragma once
#include <string>

namespace core {

class SceneNode {
public:
	explicit SceneNode(const std::string &name) : _name(name) {}
	SceneNode(const SceneNode &) = delete;
	SceneNode &operator=(const SceneNode &) = delete;
	virtual ~SceneNode() = default;
	const std::string &getName() const {
		return _name;
	};
private:
	std::string _name;	
};

}
