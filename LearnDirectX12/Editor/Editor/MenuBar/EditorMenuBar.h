#pragma once
#include <unordered_map>
#include <functional>
#include <optional>

namespace ED {

struct Menu {
	struct MenuItemGroup {
		std::string groupName;
		std::function<void()> menuItems;
	};

	MenuItemGroup *getSubItemGroupByName(const std::string &groupName);
	void removeSubItemGroupByName(const std::string &groupName);
	MenuItemGroup *addSubItemGroup(const std::string &groupName);
public:
	std::string name;
	std::vector<MenuItemGroup> subItemGroups;
};

class EditorMenuBar {
public:
	EditorMenuBar() = default;
	explicit EditorMenuBar(const std::vector<std::string> &items);
	Menu *registerBarItem(const std::string &name);
	void unregisterBarItem(const std::string &name);
	size_t getMenuBarItemsSize() const;
	Menu *getBarItem(const std::string &name);
	void drawMenuBar() const;
private:
	std::vector<Menu> _menuBarItems;
};

}
