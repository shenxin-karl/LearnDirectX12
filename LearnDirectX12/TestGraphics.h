#pragma once
#include "Graphics.h"

class TestGraphics : public Graphics {
	using Base = Graphics;
public:
	TestGraphics() = default;
	virtual void initialize() override;
	virtual void tick(GameTimer &dt) override;
	virtual void update() override;
	virtual void draw() override;
	virtual void onResize() override;
	virtual void handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
	virtual ~TestGraphics() override = default;
public:

};

