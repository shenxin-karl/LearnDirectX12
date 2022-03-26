#pragma once
#include "ConstantBuffer.h"
#include <memory>

namespace dx12lib {

template<typename T>
concept StructuredConstantBufferConcept = std::is_class_v<T> && !std::is_union_v<T>;

template<StructuredConstantBufferConcept T>
class StructuredConstantBuffer;

template<StructuredConstantBufferConcept T>
using GPUStructCBPtr = std::shared_ptr<StructuredConstantBuffer<T>>;

template<typename T>
class MappedPtr {
public:
	MappedPtr(const MappedPtr &) = delete;
	MappedPtr(MappedPtr &&) = delete;
	MappedPtr &operator=(const MappedPtr &) = delete;
	MappedPtr &operator=(MappedPtr &&) = delete;
	T *operator->() {
		return _ptr;
	}
	const T *operator->() const {
		return _ptr;
	}
	T &operator*() {
		return *_ptr;
	}
	const T &operator*() const {
		return *_ptr;
	}
	explicit operator bool() const noexcept {
		return _ptr != nullptr;
	}
	friend bool operator==(const MappedPtr &lhs, std::nullptr_t) noexcept {
		return lhs._ptr == nullptr;
	}
	friend bool operator!=(const MappedPtr &lhs, std::nullptr_t) noexcept {
		return lhs._ptr != nullptr;
	}
private:
	template<StructuredConstantBufferConcept T>
	friend class StructuredConstantBuffer;
	MappedPtr(T *ptr) : _ptr(ptr) {}
private:
	T *_ptr;
};

template<StructuredConstantBufferConcept T>
class StructuredConstantBuffer {
public:
	StructuredConstantBuffer(const void *pData, std::shared_ptr<ConstantBuffer> pConstantBuffer) 
	: _pConstantBuffer(pConstantBuffer) 
	{
		assert(pConstantBuffer != nullptr);
		setDirty();
		if (pData != nullptr)
			std::memcpy(&_buffer, pData, sizeof(T));
		else
			std::memset(&_buffer, 0, sizeof(T));
	}
	StructuredConstantBuffer(const StructuredConstantBuffer &) = delete;
	StructuredConstantBuffer(StructuredConstantBuffer &&) = delete;
	StructuredConstantBuffer &operator=(const StructuredConstantBuffer &) = delete;
	StructuredConstantBuffer &operator=(StructuredConstantBuffer &&) = delete;

	friend void swap(StructuredConstantBuffer &lhs, StructuredConstantBuffer &rhs) noexcept {
		std::swap(lhs._pConstantBuffer, rhs._pConstantBuffer);
	}

	MappedPtr<T> map() {
		setDirty();
		return MappedPtr<T>(&_buffer);
	}

	MappedPtr<const T> cmap() const {
		return MappedPtr<const T>(&_buffer);
	}

	void setDirty() {
		for (std::size_t i = 0; i < kFrameResourceCount; ++i)
			_bufferDirty.set(i, true);
	}

	void padding(const T &data) {
		setDirty();
		_buffer = data;
	}

	void padding(const void *pData) {
		assert(pData != nullptr);
		setDirty();
		std::memset(&_buffer, pData, sizeof(T));
	}

	void updateConstantBuffer() {
		auto frameIndex = _pConstantBuffer->getFrameIndex();
		if (!_bufferDirty.test(frameIndex))
			return;
		_pConstantBuffer->updateConstantBuffer(&_buffer, sizeof(T), 0);
		_bufferDirty.set(frameIndex, false);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getConstantBufferView() const {
		return _pConstantBuffer->getConstantBufferView();
	}

	std::shared_ptr<ConstantBuffer> getConstantBuffer() const {
		return _pConstantBuffer;
	}
private:
	std::bitset<kFrameResourceCount> _bufferDirty;
	std::shared_ptr<ConstantBuffer>  _pConstantBuffer;
	T _buffer;
};

}

using dx12lib::GPUStructCBPtr;