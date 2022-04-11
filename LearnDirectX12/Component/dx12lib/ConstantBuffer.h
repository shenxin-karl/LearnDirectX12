#pragma once
#include "dx12libStd.h"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class UploadBuffer;

template<typename T>
class CBufferVisitor {
public:
	CBufferVisitor(T *ptr) : _ptr(ptr) {}
	CBufferVisitor(const CBufferVisitor &) = delete;
	CBufferVisitor(CBufferVisitor &&other) = default;
	decltype(auto) operator->() const {
		return _ptr;
	}
private:
	T * const _ptr;
};


class ConstantBuffer : public IConstantBuffer {
protected:
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, std::size_t sizeInByte);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	BYTE *getMappedPtr();
	const BYTE *getMappedPtr() const;
	size_t getConstantBufferSize() const noexcept override;
	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const override;

	template<typename T>
	CBufferVisitor<T> visit() {
		assert(sizeof(T) <= getConstantBufferSize());
		return CBufferVisitor<T>(reinterpret_cast<T *>(getMappedPtr()));
	}

	template<typename T>
	CBufferVisitor<const T> visit() const {
		assert(sizeof(T) <= getConstantBufferSize());
		return CBufferVisitor<T>(getMappedPtr());
	}
private:
	DescriptorAllocation _CBV;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}
