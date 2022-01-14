#pragma once
#include <mutex>
#include <queue>

namespace dx12lib {

template<typename T>
class ThreadSafeQueue;

template<typename T>
void swap(ThreadSafeQueue<T> &lhs, ThreadSafeQueue<T> &rhs) noexcept;

template<typename T>
class ThreadSafeQueue {
	ThreadSafeQueue() = default;

	ThreadSafeQueue(const ThreadSafeQueue &other) : _queue(other._queue), _mutex(other._mutex) {
	}

	ThreadSafeQueue(ThreadSafeQueue &&other) noexcept : ThreadSafeQueue() {
		swap(*this, other);
	}

	ThreadSafeQueue &operator=(const ThreadSafeQueue &other) {
		ThreadSafeQueue tmp = other;
		swap(*this, tmp);
		return *this;
	}

	ThreadSafeQueue &operator=(ThreadSafeQueue &&other) noexcept {
		ThreadSafeQueue tmp;
		swap(*this, tmp);
		swap(*this, other);
		return *this;
	}

	friend void swap(ThreadSafeQueue &lhs, ThreadSafeQueue &rhs) noexcept {
		using std::swap;
		swap(lhs._queue, rhs._queue);
		swap(lhs._mutex, rhs._mutex);
	}

	void push(const T &val) {
		std::lock_guard lock(_mutex);
		_queue.push(val);
	}

	void push(T &&val) {
		std::lock_guard lock(_mutex);
		_queue.push(std::move(val));
	}

	bool empty() const noexcept	{
		std::lock_guard lock(_mutex);
		return _queue.empty();
	}

	std::size_t size() const noexcept {
		std::lock_guard lock(_mutex);
		return _queue.size();
	}

	bool tryPop(T &ret) {
		std::lock_guard lock(_mutex);
		if (_queue.empty())
			return false;
		
		ret = _queue.front();
		_queue.pop();
		return true;
	}
private:
	std::queue<T> _queue;
	mutable std::mutex _mutex;
};


}