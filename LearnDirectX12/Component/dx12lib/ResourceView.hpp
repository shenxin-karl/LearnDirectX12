#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"

namespace dx12lib {

enum class ViewType {
	RenderTarget	= 0,
	DepthStencil	= 1,
	ConstantBuffer  = 2,
	ShaderResource	= 3,
	UnorderedAccess = 4,
};

template<ViewType Type>
class _ResourceView {
public:
	_ResourceView() = default;
	_ResourceView(const _ResourceView &) = default;
	_ResourceView(_ResourceView &&) = default;
	_ResourceView &operator=(const _ResourceView &) = default;
	_ResourceView &operator=(_ResourceView &&) = default;
	explicit _ResourceView(const DescriptorAllocation &descriptor, size_t offset = 0)
	: _offset(offset), _descriptor(descriptor)
	{
		assert(offset < descriptor.getNumHandle());
	}
public:
	friend class CommandList;
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUDescriptorHandle() const {
		return _descriptor.getCPUHandle(_offset);
	}
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const {
		return getCPUDescriptorHandle();
	}
private:
	size_t _offset = 0;
	DescriptorAllocation _descriptor;
};

using RenderTargetView = _ResourceView<ViewType::RenderTarget>;
using DepthStencilView = _ResourceView<ViewType::DepthStencil>;
using ConstantBufferView = _ResourceView<ViewType::ConstantBuffer>;
using ShaderResourceView = _ResourceView<ViewType::ShaderResource>;
using UnorderedAccessView = _ResourceView<ViewType::UnorderedAccess>;


template<typename T>
class ViewManager {
public:
	ViewManager() : _viewIndices({ kMaxMipLevel }) {
	}

	bool exist(size_t mipSlice) const noexcept {
		assert(mipSlice < kMaxMipLevel);
		return _viewIndices[mipSlice] != kMaxMipLevel;
	}

	void set(size_t mipSlice, const T &view) noexcept {
		assert(mipSlice < kMaxMipLevel);
		uint32_t index = static_cast<uint32_t>(_viewArray.size());
		_viewIndices[mipSlice] = index;
		_viewArray.emplace_back(view);
	}

	const T &get(size_t mipSlice) const noexcept {
		assert(mipSlice < kMaxMipLevel);
		assert(_viewIndices[mipSlice] != kMaxMipLevel);
		return _viewArray[_viewIndices[mipSlice]];
	}
private:
	constexpr static size_t kMaxMipLevel = 16;
	std::vector<T> _viewArray;
	std::array<uint32_t, kMaxMipLevel> _viewIndices;
};

}
