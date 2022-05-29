#pragma once
#include <memory>
#include "Editor/IEditorItem.h"

namespace ED {

class SceneManager;
class HierarchyWindow : public IEditorWindow {
public:
	HierarchyWindow(std::shared_ptr<SceneManager> pSceneMgr);
	void showWindow() override;
	bool *getOpenFlagPtr() override;
private:
	bool _openHierarchy = true;
	std::shared_ptr<SceneManager> _pSceneMgr;
};

}
