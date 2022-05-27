#pragma once

namespace ED {

class IEditorWindow {
public:
	virtual void showWindow() = 0;
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

