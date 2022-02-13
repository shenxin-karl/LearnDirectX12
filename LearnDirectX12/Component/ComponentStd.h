#pragma once
#include <Windows.h>
#include <source_location>
#include <comdef.h>
#include <sstream>

inline void ThrowIfFailed(HRESULT hr, const std::source_location &sl = std::source_location::current()) {
	if (FAILED(hr)) {
		_com_error err(hr);
		std::stringstream sbuf;
		sbuf << "[file]: " << sl.file_name() << std::endl;
		sbuf << "[line]: " << sl.line() << std::endl;
		sbuf << "[function]: " << sl.function_name() << std::endl;
		sbuf << "[message]: " << err.ErrorMessage();
		throw std::exception(sbuf.str().c_str());
	}
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

