#pragma once
#include <string>

namespace core {

class SceneNode {
public:
	explicit SceneNode(const std::string &name);
	SceneNode(const SceneNode &) = delete;
	SceneNode &operator=(const SceneNode &) = delete;
	virtual ~SceneNode() = default;
	const std::string &getName() const;
private:
	std::string _name;	
};

}
