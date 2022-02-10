#pragma once
#include "ConstantBuffer.h"

namespace dx12lib {

template<typename T>
concept StructConstantBufferConcept = std::is_class_v<T> && std::is_trivial_v<T> && !std::is_union_v<T>;

template<StructConstantBufferConcept T>
class StructConstantBuffer;

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
	template<StructConstantBufferConcept T>
	friend class StructConstantBuffer;
	MappedPtr(T *ptr) : _ptr(ptr) {}
private:
	T *_ptr;
};

template<StructConstantBufferConcept T>
class StructConstantBuffer {
public:
	StructConstantBuffer(std::shared_ptr<ConstantBuffer> pConstantBuffer) : _pConstantBuffer(pConstantBuffer) {
	}
	StructConstantBuffer(const StructConstantBuffer &) = default;
	StructConstantBuffer(StructConstantBuffer &&) = default;
	StructConstantBuffer &operator=(const StructConstantBuffer &) = default;
	StructConstantBuffer &operator=(StructConstantBuffer &&) = default;
	T *operator->() {
		return reinterpret_cast<T *>(_pConstantBuffer->getMappedPtr());
	}
	T &operator*() {
		return *reinterpret_cast<T *>(_pConstantBuffer->getMappedPtr());
	}
	const T *operator->() const {
		return reinterpret_cast<const T *>(_pConstantBuffer->getMappedPtr());
	}
	const T &operator*() const {
		return *reinterpret_cast<const T *>(_pConstantBuffer->getMappedPtr());
	}
	explicit operator bool() const noexcept {
		return _pConstantBuffer == nullptr || _pConstantBuffer->getMappedPtr() == nullptr;
	}
	friend bool operator==(const StructConstantBuffer &lhs, std::nullptr_t) noexcept {
		return lhs.operator bool();
	}
	friend bool operator!=(const StructConstantBuffer &lhs, std::nullptr_t) noexcept {
		return !lhs.operator bool();
	}
	friend void swap(StructConstantBuffer &lhs, StructConstantBuffer &rhs) noexcept {
		std::swap(lhs._pConstantBuffer, rhs._pConstantBuffer);
	}
	MappedPtr<T> mapped() {
		return MappedPtr<T>(_pConstantBuffer->getMappedPtr());
	}
	MappedPtr<const T> mapped() const {
		return MappedPtr<const T>(_pConstantBuffer->getMappedPtr());
	}
private:
	std::shared_ptr<ConstantBuffer> _pConstantBuffer;
};

}