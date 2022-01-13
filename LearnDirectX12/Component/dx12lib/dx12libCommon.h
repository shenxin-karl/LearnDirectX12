#pragma once
#define NOMINMAX
#include <cassert>
#include <cstdint>
#include <wrl.h>
#include <exception>
#include "D3Dx12.h"
#include <comdef.h>
#include <Windows.h>
#include <d3dcompiler.h>

namespace dx12lib {

namespace WRL = Microsoft::WRL;

using uint8 = std::uint8_t;
using uin16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int8 = std::int8_t;
using in16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

inline void _ThrowIfFailedImpl(const char *file, int line, HRESULT hr) {
	if (FAILED(hr)) {
		_com_error err(hr);
		throw std::exception(err.ErrorMessage());
	}
}

#define ThrowIfFailed(hr) (_ThrowIfFailedImpl(__FILE__, __LINE__, hr))

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

enum class CommandQueueType {
	Direct,
	Compute,
	Copy,
};

constexpr static std::size_t kComandQueueTypeCount = 3;
constexpr static std::size_t kSwapChainBufferCount = 2;


}