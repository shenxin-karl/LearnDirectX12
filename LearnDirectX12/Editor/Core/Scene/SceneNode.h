#pragma once
#include <string>
#include "Editor/IEditorItem.h"

namespace ED {

class SceneNode : public IEditorItem {
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
