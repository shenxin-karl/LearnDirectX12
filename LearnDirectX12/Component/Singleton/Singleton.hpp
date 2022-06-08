#pragma once
#include <memory>

namespace com {

template<typename T>
class Singleton {
	static inline std::shared_ptr<T> sPSingleton;
public:
	static T *instance() noexcept {
		return sPSingleton.get();
	}

	template<typename... Args>
	static void emplace(Args&&... args) {
		sPSingleton = std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename U> requires(std::is_base_of_v<T, U>)
	static void emplace(std::shared_ptr<U> &&ptr) {
		sPSingleton = std::move(ptr);
	}
};

}