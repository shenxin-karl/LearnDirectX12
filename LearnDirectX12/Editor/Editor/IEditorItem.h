#pragma once

namespace ED {

class IEditorWindow {
public:
	virtual void showWindow() = 0;
	virtual bool *getOpenFlagPtr() = 0;
};

class IEditorMenu {
public:
	virtual void showMenu() = 0;
};

class IEditorItem {
public:
	virtual void showInspector() = 0;
};

class IComponent {
public:
	virtual void showProperty() = 0;
};



}

