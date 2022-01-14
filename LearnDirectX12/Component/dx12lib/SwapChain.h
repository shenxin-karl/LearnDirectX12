#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class Device;
class CommandQueue;
class Texture;

class SwapChain {
public:
	SwapChain(std::shared_ptr<Device> pDevice, 
		HWND hwnd, 
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat
	);

	void resize(uint32 width, uint32 height);
	std::shared_ptr<Texture> getRenderTarget() const;
	std::shared_ptr<Texture> getDepthStencil() const;
	DXGI_FORMAT getRenderTargetFormat() const;
	DXGI_FORMAT getDepthStencilFormat() const;
	UINT present();
private:
	void updateBuffer();
private:
	HWND _hwnd;
	uint32	_width;
	uint32  _height;
	DXGI_FORMAT  _renderTargetFormat;
	DXGI_FORMAT  _depthStendilFormat;
	std::size_t  _currentBackBufferIndex;
	std::shared_ptr<Texture>  _pSwapChainBuffer[kSwapChainBufferCount];
	std::shared_ptr<Texture>  _pDepthStencilBuffer;
	WRL::ComPtr< IDXGISwapChain>  _pSwapChain;
	std::weak_ptr<CommandQueue> _pCommandQueue;
	std::weak_ptr<Device> _pDevice;
};

}