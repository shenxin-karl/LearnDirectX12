#include "Editor/Editor.h"
#include "GameTimer/GameTimer.h"
#include <iostream>
#include <memory>
#include "Reflection/StaticReflection.hpp"
#include "Reflection/DynamicReflection.h"

template<typename T>
void test() {
	using type = std::decay_t<const char *>();
	if constexpr (std::is_array_v<T>) {
		std::cout << typeid(decltype(std::declval<T>()[0])).name() << std::endl;
	}

}

struct AAA {
	int aaa[10];
	double bbb;
};

DEFINE_REFLECT(AAA, aaa, bbb);

int main() {

	std::cout << typeid(decltype(std::declval<AAA>().aaa)).name() << std::endl;
	std::cout << std::is_floating_point_v<double> << std::endl;

	refl::DynamicReflItem<AAA> dynamicRefl;

	std::shared_ptr<com::GameTimer> pGameTimer = std::make_shared<com::GameTimer>();
	ED::Editor editor;
	try {
		editor.initialize();
		editor.setGameTimer(pGameTimer);

		while (editor.isRunning()) {
			pGameTimer->startNewFrame();
			editor.beginTick(pGameTimer);
			editor.tick(pGameTimer);
			editor.endTick(pGameTimer);
		}
		editor.destroy();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		MessageBox(nullptr, e.what(), "Error", MB_OK | MB_ICONHAND);
	}
	return 0;
}
