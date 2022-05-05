#pragma once
#include "D3D/d3dutil.h"
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>
#include <Math/MathStd.hpp>

namespace d3d {

using namespace Math;

class RenderTarget : public NonCopyable {
public:
	RenderTarget(std::shared_ptr<dx12lib::RenderTarget2D> pRenderTarget2D, std::shared_ptr<dx12lib::DepthStencil2D> pDepthStencil2D);
	explicit RenderTarget(std::shared_ptr<dx12lib::SwapChain> pSwapChain);
	RenderTarget(dx12lib::GraphicsContextProxy pGraphicsCtx,
		size_t width, 
		size_t height, 
		DXGI_FORMAT renderTargetFormat, 
		DXGI_FORMAT depthStencilFormat
	);
	std::shared_ptr<dx12lib::RenderTarget2D> getRenderTarget2D() const;
	std::shared_ptr<dx12lib::DepthStencil2D> getDepthTarget2D() const;
	void clear(dx12lib::GraphicsContextProxy pCommonCtx, const Math::float4 &color, float depth = 1.f, UINT stencil = 0);
	void bind(dx12lib::GraphicsContextProxy pCommonCtx);
	void unbind(dx12lib::CommonContextProxy pCommonCtx);
	D3D12_VIEWPORT getViewport() const;
	D3D12_RECT getScissorRect() const;
	float2 getRenderTargetSize() const;
	float2 getInvRenderTargetSize() const;
	~RenderTarget();
private:
	size_t _width;
	size_t _height;
	bool   _isBinding = false;
	std::shared_ptr<dx12lib::RenderTarget2D> _pRenderTarget2D;
	std::shared_ptr<dx12lib::DepthStencil2D> _pDepthStencil2D;
};

}
