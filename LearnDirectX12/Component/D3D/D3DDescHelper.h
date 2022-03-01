#pragma once
#include "D3Dx12.h"
#include <functional>


namespace d3dutil {

// render target blend helper
enum class RenderTargetBlendPreset {
	ADD,
	SUBTRACT,
	REV_SUBTRACT,
	MULTIPILES,
	ALPHA,
	REV_ALPHA,
};

struct RenderTargetBlendDescHelper : public D3D12_RENDER_TARGET_BLEND_DESC {
public:
	RenderTargetBlendDescHelper(RenderTargetBlendPreset preset);
private:
	void initBlendOpAdd();
	void initBlendOpSubtract();
	void initBlendOpRevSubtract();
	void initBlendOpMultipiles();
	void initBlendOpEnable();
	void initBlendOpAlpha();
	void initBlendOpRevAlpha();
};

}