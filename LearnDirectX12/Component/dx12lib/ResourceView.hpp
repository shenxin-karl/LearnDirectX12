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
	_ResourceView(const _ResourceView &) = default;
	_ResourceView(_ResourceView &&) = default;
	_ResourceView &operator=(const _ResourceView &) = default;
	_ResourceView &operator=(_ResourceView &&) = default;
	explicit _ResourceView(size_t offset, const DescriptorAllocation &descriptor)
	: _offset(offset), _descriptor(descriptor)
	{
		assert(offset < descriptor.getNumHandle());
	}
private:
	friend class CommandList;
	D3D12_CPU_DESCRIPTOR_HANDLE getCPUDescriptorHandle() const {
		return _descriptor.getCPUHandle(_offset);
	}
	operator D3D12_CPU_DESCRIPTOR_HANDLE() const {
		return getCPUDescriptorHandle();
	}
private:
	size_t _offset;
	DescriptorAllocation _descriptor;
};

using RenderTargetView = _ResourceView<ViewType::RenderTarget>;
using DepthStencilView = _ResourceView<ViewType::DepthStencil>;
using ConstantBufferView = _ResourceView<ViewType::ConstantBuffer>;
using ShaderResourceView = _ResourceView<ViewType::ShaderResource>;
using UnorderedAccessView = _ResourceView<ViewType::UnorderedAccess>;

}
