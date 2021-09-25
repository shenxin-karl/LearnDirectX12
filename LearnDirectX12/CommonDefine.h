#pragma once
#define NOMINMAX
#include <Windows.h>
#include <format>


#define SAssert(res, msg) \
do {	\
	if (!(res)) { \
		MessageBox(nullptr, std::format("file:{}:line:{}, ErrorMessage:{}", __FILE__, __LINE__, (msg)).c_str(), \
		"Error", MB_OK | MB_ICONERROR);	\
		exit(EXIT_FAILURE);	\
	}	\
} while(0)

