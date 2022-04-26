#pragma once
#include "dx12libStd.h"
#include "CBufferVisitor.hpp"
#include "DescriptorAllocation.h"
#include "IResource.h"

namespace dx12lib {

class UploadBuffer;

class ConstantBuffer : public IConstantBuffer {
protected:
	ConstantBuffer(std::weak_ptr<Device> pDevice, const void *pData, std::size_t sizeInByte);
public:
	WRL::ComPtr<ID3D12Resource> getD3DResource() const override;
	void updateConstantBuffer(const void *pData, size_t sizeInByte, size_t offset = 0) override;
	BYTE *getMappedPtr();
	const BYTE *getMappedPtr() const;
	size_t getConstantBufferSize() const noexcept override;
	ConstantBufferView getConstantBufferView() const override;

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
	ConstantBufferView _constantBufferView;
	std::unique_ptr<UploadBuffer> _pUploadBuffer;
};

}
