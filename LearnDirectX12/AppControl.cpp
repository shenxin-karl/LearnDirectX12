#include "AppControl.h"
#include "TestGraphics.h"


std::unique_ptr<Graphics> TestAppControl::createGraphics() {
	return std::make_unique<TestGraphics>();
}