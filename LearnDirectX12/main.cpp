#include <iostream>
#include "ExceptionBase.h"
#include "D3DApp.h"
#include "AppControl.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {

	auto control = std::make_shared<TestAppControl>();
	D3DApp::emplace(control);
	return D3DApp::instance()->start();
}