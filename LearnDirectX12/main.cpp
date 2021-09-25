#include <Windows.h>
#include "window.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {

	Window window(800, 600, "LearnDirectX12");
	while (!window.shouldClose()) {
		window.pollEvent();
	}
	return window.getReturnCode();
}