#pragma once
#include <dx12lib/dx12libStd.h>
#include <dx12lib/Context/ContextProxy.hpp>

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
	std::shared_ptr<RenderTarget2D> getRenderTarget() const;
	std::shared_ptr<DepthStencil2D> getDepthStencil() const;
	void present();
private:
	std::shared_ptr<RenderTarget2D> getCurrentBackBuffer() const;
	void updateBuffer(DirectContextProxy pDirectContext);
private:
	HWND	_hwnd;
	uint32	_width;
	uint32	_height;
	DXGI_FORMAT	_renderTargetFormat;
	DXGI_FORMAT	_depthStencilFormat;
	std::size_t	_currentBackBufferIndex;
	std::weak_ptr<Device>			_pDevice;
	WRL::ComPtr<IDXGISwapChain>		_pSwapChain;
	std::shared_ptr<DepthStencil2D>	_pDepthStencil2D;
	std::shared_ptr<RenderTarget2D>	_pSwapChainBuffer[kSwapChainBufferCount];
};

}