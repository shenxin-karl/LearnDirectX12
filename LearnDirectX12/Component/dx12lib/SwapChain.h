#pragma once
#include "dx12libStd.h"
#include "ContextProxy.hpp"

namespace dx12lib {

class SwapChain {
protected:
	SwapChain(std::weak_ptr<Device> pDevice, 
		HWND hwnd, 
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat
	);
public:
	void resize(DirectContextProxy pDirectContext, uint32 width, uint32 height);
	DXGI_FORMAT getRenderTargetFormat() const;
	DXGI_FORMAT getDepthStencilFormat() const;
	void present();
	std::shared_ptr<RenderTarget> getRenderTarget() const;
private:
	std::shared_ptr<RenderTargetBuffer> getCurrentBackBuffer() const;
	void updateBuffer(DirectContextProxy pDirectContext);
private:
	HWND	_hwnd;
	uint32	_width;
	uint32	_height;
	DXGI_FORMAT	_renderTargetFormat;
	DXGI_FORMAT	_depthStendilFormat;
	std::size_t	_currentBackBufferIndex;
	std::weak_ptr<Device>				_pDevice;
	WRL::ComPtr<IDXGISwapChain>			_pSwapChain;
	std::shared_ptr<RenderTarget>		_pRenderTarget;
	std::shared_ptr<RenderTargetBuffer> _pSwapChainBuffer[kSwapChainBufferCount];
	std::shared_ptr<DepthStencilBuffer> _pDepthStencilBuffer;
};

}