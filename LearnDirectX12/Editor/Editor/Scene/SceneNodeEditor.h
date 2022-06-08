#pragma once

namespace ED {

class ISceneNodeEditor {
public:
	ISceneNodeEditor() = default;
	virtual ~ISceneNodeEditor() = default;
	virtual void showInspector() = 0;
};

}
