#pragma once
#include <mutex>
#include <queue>
#include <shared_mutex>

namespace dx12lib {

template<typename T>
class ThreadSafeQueue;

template<typename T>
void swap(ThreadSafeQueue<T> &lhs, ThreadSafeQueue<T> &rhs) noexcept;

template<typename T>
class ThreadSafeQueue {
public:
	ThreadSafeQueue() = default;

	ThreadSafeQueue(const ThreadSafeQueue &other) : _queue(other._queue) {
	}

	ThreadSafeQueue(ThreadSafeQueue &&other) noexcept : ThreadSafeQueue() {
		std::lock_guard lock(other._mutex);
		swap(*this, other);
	}

	ThreadSafeQueue &operator=(const ThreadSafeQueue &other) {
		std::shared_lock lock(other._mutex);
		ThreadSafeQueue tmp = other;
		lock.unlock();

		std::lock_guard currLock(_mutex);
		swap(*this, tmp);
		return *this;
	}

	ThreadSafeQueue &operator=(ThreadSafeQueue &&other) noexcept {
		ThreadSafeQueue tmp;
		std::lock_guard lock1(other._mutex);
		std::lock_guard lock2(other._mutex);
		swap(*this, tmp);
		swap(*this, other);
		return *this;
	}

	void push(const T &val) {
		std::unique_lock lock(_mutex);
		_queue.push(val);
	}

	void push(T &&val) {
		std::unique_lock lock(_mutex);
		_queue.push(std::move(val));
	}

	bool empty() const noexcept	{
		std::shared_lock lock(_mutex);
		return _queue.empty();
	}

	std::size_t size() const noexcept {
		std::shared_lock lock(_mutex);
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
	friend void swap(ThreadSafeQueue &lhs, ThreadSafeQueue &rhs) noexcept {
		using std::swap;
		swap(lhs._queue, rhs._queue);
		//swap(lhs._mutex, rhs._mutex);
	}
private:
	std::queue<T> _queue;
	mutable std::shared_mutex _mutex;
};


}