#pragma once
#include "Core/Scene/SkyBoxNode.h"
#include "Editor/Scene/SceneNodeEditor.h"

namespace ED {

class SkyBoxNodeEditor : public core::SkyBoxNode {
public:
	explicit SkyBoxNodeEditor(const std::string &name);
	void showInspector();
private:
	std::string _skyBoxPath;
};

}
