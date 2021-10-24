#pragma once
#include <memory>

class Graphics;
class AppControl {
public:
	virtual ~AppControl() = default;
	virtual std::unique_ptr<Graphics> createGraphics() = 0;
};

class TestAppControl : public AppControl {
public:
	virtual ~TestAppControl() = default;
	virtual std::unique_ptr<Graphics> createGraphics() override;
};