#pragma once
#define NOMINMAX
#include <Windows.h>
#include <format>


#define SAssertImpl(res, msg, file, line) \
do {	\
	if (!(res)) { \
		MessageBox(nullptr, std::format("file:{}:line:{}, ErrorMessage:{}", (file), (line), (msg)).c_str(), \
		"Error", MB_OK | MB_ICONERROR);	\
		exit(EXIT_FAILURE);	\
	}	\
} while(0)


#define SAssert(res, msg) SAssertImpl(res, msg, __FILE__, __LINE__)