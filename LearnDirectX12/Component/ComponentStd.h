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

#define USE_RVPTR_FUNC_IMPL
template<typename T>
const T *_rightValuePtr(const T &val) {
	return &val;
}

template<typename T>
constexpr auto RVPtr(T &&val) {
#ifdef USE_RVPTR_FUNC_IMPL
	return _rightValuePtr(val);
#else
	return &val;
#endif
}

