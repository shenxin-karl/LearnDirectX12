#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Descriptor/DescriptorAllocation.h>

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
	_ResourceView(_ResourceView &&) noexcept = default;
	_ResourceView &operator=(const _ResourceView &) = default;
	_ResourceView &operator=(_ResourceView &&) noexcept = default;
	explicit _ResourceView(const DescriptorAllocation &descriptor, size_t offset = 0)
	: _offset(offset), _descriptor(descriptor)
	{
		assert(offset < descriptor.getNumHandle());
	}
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

template<typename T>
class _BufferView {
	friend class CommandList;
public:
	template<typename ... Args> requires(sizeof...(Args) > 0)
	_BufferView(Args&&...args) : _view(std::forward<Args>(args)...) {}
	_BufferView(D3D12_VERTEX_BUFFER_VIEW view) : _view(view) {}
	_BufferView(const _BufferView &) = default;
	_BufferView(_BufferView &&) noexcept = default;
	_BufferView &operator=(const _BufferView &) = default;
	_BufferView &operator=(_BufferView &&) noexcept = default;
	operator T() const {
		return _view;
	}
	T *operator&() {
		return &_view;
	}
	const T *operator&() const {
		return &_view;
	}
private:
	T _view;
};

using RenderTargetView = _ResourceView<ViewType::RenderTarget>;
using DepthStencilView = _ResourceView<ViewType::DepthStencil>;
using ConstantBufferView = _ResourceView<ViewType::ConstantBuffer>;
using ShaderResourceView = _ResourceView<ViewType::ShaderResource>;
using UnorderedAccessView = _ResourceView<ViewType::UnorderedAccess>;
using VertexBufferView = _BufferView<D3D12_VERTEX_BUFFER_VIEW>;
using IndexBufferView = _BufferView<D3D12_INDEX_BUFFER_VIEW>;

template<typename T>
class ViewManager {
public:
	ViewManager() {
		for (auto &idx : _viewIndices)
			idx = kMaxMipLevel;
	}
	ViewManager(const ViewManager &) = delete;
	ViewManager(ViewManager &&) = default;
	ViewManager &operator=(const ViewManager &) = delete;
	ViewManager &operator=(ViewManager &&) = default;

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
