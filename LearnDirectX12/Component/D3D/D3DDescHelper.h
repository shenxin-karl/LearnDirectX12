#pragma once
#include "D3Dx12.h"
#include <functional>


namespace d3d {

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


// SP: depth stencil success
enum class DepthStendilOpPreset {
	SP_REPLACE,
	SP_KEEP,
	SP_ZERO,
	SP_INCR,
	SP_DECR,
};

struct DepthStencilOpDescHelper : public D3D12_DEPTH_STENCILOP_DESC {
	DepthStencilOpDescHelper(DepthStendilOpPreset preset, D3D12_COMPARISON_FUNC func = D3D12_COMPARISON_FUNC_ALWAYS);
};

}