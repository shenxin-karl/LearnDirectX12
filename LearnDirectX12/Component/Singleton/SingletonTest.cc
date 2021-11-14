#include <iostream>
#include <format>
#include "Singleton.hpp"

namespace com {

class A : public Singleton<A> {
public:
	int v1_ = 0;
	int v2_ = 1;
public:
	A(int v1, int v2) : v1_(v1), v2_(v2) {}
};

int main() {
	A::emplace(10, 20);
	std::cout << std::format("A::v1 = {}, A::v2 = {}",
		A::instance()->v1_,
		A::instance()->v2_
	);
}
}
