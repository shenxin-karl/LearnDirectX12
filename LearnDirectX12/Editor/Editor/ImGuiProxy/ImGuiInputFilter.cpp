#include <utility>
#include "Editor/ImGuiProxy/ImGuiInputFilter.h"
#include "Editor/ImGuiProxy/ImGuiInput.h"

namespace ED {

ImGuiInputFilter::ImGuiInputFilter(const std::string &windowName) : _isFocus(false), _windowName(windowName) {
}

ImGuiInputFilter::ImGuiInputFilter(std::shared_ptr<ImGuiInput> pImGuiInput, const std::string &windowName)
: _isFocus(false), _workRect(0.f, 0.f, 0.f, 0.f)
, _windowName(windowName), _pImGuiInput(std::move(pImGuiInput))
{
}

bool ImGuiInputFilter::isPassKeyBoardFilter() const {
	return _isFocus;
}

bool ImGuiInputFilter::isPassMouseFilter() const {
	ImVec2 lastMousePos = _pImGuiInput->getLastMousePos();
	int flag = 0;
	flag |= static_cast<int>(lastMousePos.x < _workRect.Min.x);
	flag |= static_cast<int>(lastMousePos.x > _workRect.Max.x);
	flag |= static_cast<int>(lastMousePos.y < _workRect.Min.y);
	flag |= static_cast<int>(lastMousePos.y > _workRect.Max.y);
	return flag == 0;
}

void ImGuiInputFilter::update() {
	ImGuiWindow *pWindow = ImGui::FindWindowByName(_windowName.c_str());
	assert(pWindow != nullptr);

	auto *pContext = ImGui::GetCurrentContext();
	_isFocus = pContext->NavWindow->ID == pWindow->ID;
	_workRect = pWindow->WorkRect;
}

const std::string &ImGuiInputFilter::getWindowName() const {
	return _windowName;
}

void ImGuiInputFilter::setImGuiInput(std::shared_ptr<ImGuiInput> pImGuiInput) {
	_pImGuiInput = std::move(pImGuiInput);
}

std::shared_ptr<ImGuiInput> ImGuiInputFilter::getImGuiInput() const {
	return _pImGuiInput;
}

}
