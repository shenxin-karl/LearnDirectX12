#pragma once
#define NOMINMAX
#include <Windows.h>

namespace com {
	struct CharEvent;
	struct KeyEvent;
	struct MouseEvent;
}

namespace ED {

class IEditorWindow {
public:
	virtual void showWindow() = 0;
};

class IEditorItem {
public:
	virtual void showInspector() = 0;
	virtual void showProperty() = 0;
};

}

