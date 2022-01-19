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
#include <memory>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>
#include <queue>
#include <source_location>
#include <mutex>
#include <dxgi.h>

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

inline void ThrowIfFailed(HRESULT hr, const std::source_location &sl = std::source_location::current()) {
	if (FAILED(hr)) {
		_com_error err(hr);
		throw std::exception(err.ErrorMessage());
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

enum class CommandQueueType {
	Direct,
	Compute,
	Copy,
};

constexpr static std::size_t kComandQueueTypeCount = 3;
constexpr static std::size_t kSwapChainBufferCount = 2;
constexpr static std::size_t kFrameResourceCount = 3;

class Device;

}