#include <cassert>
#include <Imgui/imgui.h>
#include "EditorMenuBar.h"

namespace ED {

Menu::MenuItemGroup *Menu::getSubItemGroupByName(const std::string &groupName) {
	for (auto &subItemGroup : subItemGroups) {
		if (subItemGroup.groupName == groupName)
			return &subItemGroup;
	}
	return nullptr;
}

void Menu::removeSubItemGroupByName(const std::string &groupName) {
	for (auto iter = subItemGroups.begin(); iter != subItemGroups.end(); ++iter) {
		if (iter->groupName == groupName) {		
			subItemGroups.erase(iter);
			return;
		}
	}
}

Menu::MenuItemGroup *Menu::addSubItemGroup(const std::string &groupName) {
	for (auto &subItemGroup : subItemGroups) {
		if (subItemGroup.groupName == groupName) {
			return &subItemGroup;
		}
	}
	subItemGroups.emplace_back(name);
	return &subItemGroups.back();
}

EditorMenuBar::EditorMenuBar(const std::vector<std::string> &items) {
	_menuBarItems.reserve(items.size());
	for (auto &name : items)
		_menuBarItems.emplace_back(name);
}

Menu *EditorMenuBar::registerBarItem(const std::string &name) {
	for (auto &barItem : _menuBarItems) {
		if (barItem.name == name)
			return &barItem;
	}
	_menuBarItems.emplace_back(name);
	return &_menuBarItems.back();
}

void EditorMenuBar::unregisterBarItem(const std::string &name) {
	for (auto iter = _menuBarItems.begin(); iter != _menuBarItems.end(); ++iter) {
		if (iter->name == name) {
			_menuBarItems.erase(iter);
			return;
		}
	}
}

size_t EditorMenuBar::getMenuBarItemsSize() const {
	return _menuBarItems.size();
}

Menu *EditorMenuBar::getBarItem(const std::string &name) {
	for (auto &barItem : _menuBarItems) {
		if (barItem.name == name)
			return &barItem;
	}
	return nullptr;
}

void EditorMenuBar::drawMenuBar() const {
	if (_menuBarItems.empty())
		return;

	for (size_t i = 0; i < _menuBarItems.size(); ++i) {
		if (ImGui::BeginMenu(_menuBarItems[i].name.c_str())) {
			for (auto &menuItemGroup : _menuBarItems[i].subItemGroups) {
				for (auto &menuItem : menuItemGroup.menuItems)
					menuItem();
			}
			ImGui::EndMenu();
		}
	}
}

}
