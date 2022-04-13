#pragma once
#define NOMINMAX
#include <Windows.h>
#include <source_location>
#include <sstream>
#include "D3D/D3DException.h"

inline void ThrowIfFailed(HRESULT hr, const std::source_location &sl = std::source_location::current()) {
	if (FAILED(hr))
		throw d3d::D3DException(hr, sl.file_name(), sl.line());
}


template<typename T>
constexpr decltype(auto) RVPtr(T &&val) {
	return &val;
}

