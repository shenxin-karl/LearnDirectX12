#include "D3DDescHelper.h"
#include <cassert>

namespace d3d {

RenderTargetBlendDescHelper::RenderTargetBlendDescHelper(RenderTargetBlendPreset preset) {
	switch (preset)
	{
	case d3d::RenderTargetBlendPreset::ADD:
		initBlendOpAdd();
		break;
	case d3d::RenderTargetBlendPreset::SUBTRACT:
		initBlendOpSubtract();
		break;
	case d3d::RenderTargetBlendPreset::REV_SUBTRACT:
		initBlendOpRevSubtract();
		break;
	case d3d::RenderTargetBlendPreset::MULTIPILES:
		initBlendOpMultipiles();
		break;
	case RenderTargetBlendPreset::ALPHA:
		initBlendOpAlpha();
		break;
	case RenderTargetBlendPreset::REV_ALPHA:
		initBlendOpRevAlpha();
		break;
	default:
		assert(false);
		break;
	}
}

void RenderTargetBlendDescHelper::initBlendOpAdd() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_ONE;
	this->DestBlend = D3D12_BLEND_ONE;
	this->BlendOp = D3D12_BLEND_OP_ADD;
	this->SrcBlendAlpha = D3D12_BLEND_ONE;
	this->DestBlendAlpha = D3D12_BLEND_ONE;
	this->BlendOpAlpha = D3D12_BLEND_OP_ADD;
}

void RenderTargetBlendDescHelper::initBlendOpSubtract() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_ONE;
	this->DestBlend = D3D12_BLEND_ONE;
	this->BlendOp = D3D12_BLEND_OP_SUBTRACT;
	this->SrcBlendAlpha = D3D12_BLEND_ONE;
	this->DestBlendAlpha = D3D12_BLEND_ONE;
	this->BlendOpAlpha = D3D12_BLEND_OP_SUBTRACT;
}

void RenderTargetBlendDescHelper::initBlendOpRevSubtract() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_ONE;
	this->DestBlend = D3D12_BLEND_ONE;
	this->BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	this->SrcBlendAlpha = D3D12_BLEND_ONE;
	this->DestBlendAlpha = D3D12_BLEND_ONE;
	this->BlendOpAlpha = D3D12_BLEND_OP_REV_SUBTRACT;
}

void RenderTargetBlendDescHelper::initBlendOpMultipiles() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_ZERO;
	this->DestBlend = D3D12_BLEND_SRC_COLOR;
	this->BlendOp = D3D12_BLEND_OP_ADD;
	this->SrcBlendAlpha = D3D12_BLEND_ZERO;
	this->DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	this->BlendOpAlpha = D3D12_BLEND_OP_ADD;
}

void RenderTargetBlendDescHelper::initBlendOpEnable() {
	this->BlendEnable = TRUE;
	this->LogicOpEnable = FALSE;
	this->LogicOp = D3D12_LOGIC_OP_CLEAR;
	this->RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
}

void RenderTargetBlendDescHelper::initBlendOpAlpha() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_SRC_ALPHA;
	this->DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	this->BlendOp = D3D12_BLEND_OP_ADD;
	this->SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	this->DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	this->BlendOpAlpha = D3D12_BLEND_OP_ADD;
}

void RenderTargetBlendDescHelper::initBlendOpRevAlpha() {
	initBlendOpEnable();
	this->SrcBlend = D3D12_BLEND_INV_SRC_ALPHA;
	this->DestBlend = D3D12_BLEND_SRC_ALPHA;
	this->BlendOp = D3D12_BLEND_OP_ADD;
	this->SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	this->DestBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	this->BlendOpAlpha = D3D12_BLEND_OP_ADD;
}

DepthStencilOpDescHelper::DepthStencilOpDescHelper(DepthStendilOpPreset preset, D3D12_COMPARISON_FUNC func) {
	this->StencilFunc = func;
	this->StencilFailOp = D3D12_STENCIL_OP_KEEP;
	this->StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	switch (preset) {
	case d3d::DepthStendilOpPreset::SP_REPLACE:
		this->StencilPassOp = D3D12_STENCIL_OP_REPLACE;
		break;
	case d3d::DepthStendilOpPreset::SP_KEEP:
		this->StencilPassOp = D3D12_STENCIL_OP_KEEP;
		break;
	case d3d::DepthStendilOpPreset::SP_ZERO:
		this->StencilPassOp = D3D12_STENCIL_OP_ZERO;
		break;
	case d3d::DepthStendilOpPreset::SP_INCR:
		this->StencilPassOp = D3D12_STENCIL_OP_INCR;
		break;
	case d3d::DepthStendilOpPreset::SP_DECR:
		this->StencilPassOp = D3D12_STENCIL_OP_DECR;
		break;
	default:
		assert(false);
		break;
	}
}


}