#pragma once
#include <memory>
#include "Editor/IEditorItem.h"

namespace ED {

class SceneManager;
class InspectorWindow : public IEditorWindow {
public:
	InspectorWindow(std::shared_ptr<SceneManager> pSceneMgr);
	void showWindow() override;
	bool *getOpenFlagPtr() override;
private:
	bool _openInspector;
	std::shared_ptr<SceneManager> _pSceneMgr;
};

}
