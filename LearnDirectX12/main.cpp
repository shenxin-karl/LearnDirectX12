#include <iostream>
#include "ExceptionBase.h"
#include "D3DApp.h"

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
) {
	D3DApp::emplace();
	return D3DApp::instance()->start();
}