#pragma once
#include "dx12libStd.h"

namespace dx12lib {

class Device;
class CommandQueue;
class Texture;
class RenderTarget;

class SwapChain {
public:
	SwapChain(std::weak_ptr<Device> pDevice, 
		HWND hwnd, 
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat
	);
	void resize(uint32 width, uint32 height);
	DXGI_FORMAT getRenderTargetFormat() const;
	DXGI_FORMAT getDepthStencilFormat() const;
	UINT present();
	std::shared_ptr<RenderTarget> getRenderTarget() const;
private:
	std::shared_ptr<Texture> getCurrentBackBuffer() const;
	void updateBuffer();
private:
	HWND                          _hwnd;
	uint32	                      _width;
	uint32                        _height;
	DXGI_FORMAT                   _renderTargetFormat;
	DXGI_FORMAT                   _depthStendilFormat;
	std::size_t                   _currentBackBufferIndex;
	std::weak_ptr<Device>         _pDevice;
	std::shared_ptr<Texture>      _pSwapChainBuffer[kSwapChainBufferCount];
	std::shared_ptr<Texture>      _pDepthStencilBuffer;
	std::shared_ptr<RenderTarget> _pRenderTarget;
	WRL::ComPtr<IDXGISwapChain>   _pSwapChain;
};

}