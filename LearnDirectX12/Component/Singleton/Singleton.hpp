#pragma once
#include <memory>

namespace com {

template<typename T>
class Singleton {
	static inline std::unique_ptr<T> singletonPtr_;
public:
	static T *instance() noexcept {
		return singletonPtr_.get();
	}

	template<typename... Args>
	static void emplace(Args&&... args) {
		singletonPtr_ = std::make_unique<T>(std::forward<Args>(args)...);
	}
};

}