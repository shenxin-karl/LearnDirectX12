#pragma once
#include <memory>
#include <string>
#include <Imgui/imgui_internal.h>

namespace ED {

class ImGuiInput;

class ImGuiInputFilter {
public:
	explicit ImGuiInputFilter(const std::string &windowName);
	ImGuiInputFilter(std::shared_ptr<ImGuiInput> pImGuiInput, const std::string &windowName);
	bool isPassKeyBoardFilter() const;
	bool isPassMouseFilter() const;
	void update();
	const std::string &getWindowName() const;
	void setImGuiInput(std::shared_ptr<ImGuiInput> pImGuiInput);
	std::shared_ptr<ImGuiInput> getImGuiInput() const;
private:
	bool _isFocus;
	ImRect _workRect;
	std::string _windowName;
	std::shared_ptr<ImGuiInput> _pImGuiInput;
};

}
