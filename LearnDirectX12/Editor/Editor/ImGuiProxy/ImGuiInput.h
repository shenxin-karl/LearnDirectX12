#pragma once
#include <vector>
#include <Imgui/imgui.h>

namespace ED {

class ImGuiInput {
public:
	ImGuiInput();
	static bool isKeyCtrlMode();
	static bool isKeyShiftMode();
	static bool isKeyAltMode();
	bool isMouseDown(ImGuiMouseButton button) const;
	bool isMouseClicked(ImGuiMouseButton button) const;
	bool isMouseReleased(ImGuiMouseButton button) const;
	bool isKeyDown(ImGuiKey key) const;
	bool isKeyPressed(ImGuiKey key) const;
	bool isKeyReleased(ImGuiKey key) const;
	float getMouseWheel() const;
	const std::vector<char> &getCharQueues() const;
	ImVec2 getLastMousePos() const;
	ImVec2 getMouseDelta() const;
	void reset();
	void update();
private:
	float _mouseWheel;
	ImVec2 _mouseDelta;
	ImVec2 _lastMousePos;
	bool _mouseDownState[ImGuiMouseButton_COUNT];
	bool _mouseClickedState[ImGuiMouseButton_COUNT];
	bool _mouseReleasedState[ImGuiMouseButton_COUNT];
	bool _keyDownState[ImGuiKey_COUNT];
	bool _keyPressedState[ImGuiKey_COUNT];
	bool _keyReleasedState[ImGuiKey_COUNT];
	std::vector<char> _charQueues;
};

}
