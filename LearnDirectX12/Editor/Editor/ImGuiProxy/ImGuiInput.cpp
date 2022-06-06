#include "Editor/ImGuiProxy/ImGuiInput.h"

namespace ED {

ImGuiInput::ImGuiInput() {
	reset();
}

bool ImGuiInput::isKeyCtrlMode() {
	return ImGui::GetIO().KeyCtrl;
}

bool ImGuiInput::isKeyShiftMode() {
	return ImGui::GetIO().KeyShift;
}

bool ImGuiInput::isKeyAltMode() {
	return ImGui::GetIO().KeyAlt;
}

bool ImGuiInput::isMouseDown(ImGuiMouseButton button) const {
	return _mouseDownState[button];
}

bool ImGuiInput::isMouseClicked(ImGuiMouseButton button) const {
	return _mouseClickedState[button];
}

bool ImGuiInput::isMouseReleased(ImGuiMouseButton button) const {
	return _mouseReleasedState[button];
}

bool ImGuiInput::isKeyDown(ImGuiKey key) const {
	return _keyDownState[key];
}

bool ImGuiInput::isKeyPressed(ImGuiKey key) const {
	return _keyPressedState[key];
}

bool ImGuiInput::isKeyReleased(ImGuiKey key) const {
	return _keyReleasedState[key];
}

float ImGuiInput::getMouseWheel() const {
	return _mouseWheel;
}

const std::vector<char> &ImGuiInput::getCharQueues() const {
	return _charQueues;
}

ImVec2 ImGuiInput::getLastMousePos() const {
	return _lastMousePos;
}

void ImGuiInput::reset() {
	_mouseWheel = 0.f;
	_mouseDelta = {  0.f, 0.f };
	std::memset(&_mouseDownState, 0, sizeof(_mouseDownState));
	std::memset(&_mouseClickedState, 0, sizeof(_mouseClickedState));
	std::memset(&_mouseReleasedState, 0, sizeof(_mouseReleasedState));
	std::memset(&_keyDownState, 0, sizeof(_keyDownState));
	std::memset(&_keyPressedState, 0, sizeof(_keyPressedState));
	std::memset(&_keyReleasedState, 0, sizeof(_keyReleasedState));
	_charQueues.clear();
}

void ImGuiInput::update() {
	reset();

	auto &io = ImGui::GetIO();
	if (io.WantCaptureMouse && ImGui::IsMousePosValid()) {
		_mouseDelta = io.MouseDelta;
		_mouseWheel = io.MouseWheel;
		_lastMousePos = io.MousePos;
		for (size_t i = 0; i < std::size(io.MouseDown); ++i) {
			_mouseDownState[i] = ImGui::IsMouseDown(i);
			_mouseClickedState[i] = ImGui::IsMouseClicked(i);
			_mouseReleasedState[i] = ImGui::IsMouseReleased(i);
		}
	}

	if (io.WantCaptureKeyboard) {
		for (ImGuiKey key = 0; key < ImGuiKey_COUNT; ++key) {
			if (key < 512 && io.KeyMap[key] != -1) 
				continue;
			_keyDownState[key] = ImGui::IsKeyDown(key);
			_keyPressedState[key] = ImGui::IsKeyPressed(key);
			_keyReleasedState[key] = ImGui::IsKeyReleased(key);
		}
	}

	if (io.WantTextInput) {
		for (auto &&inputQueueCharacter : io.InputQueueCharacters)
			_charQueues.push_back(inputQueueCharacter);
	}
}

}
