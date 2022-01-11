#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class Device;
class CommandQueue;

class SwapChain {
public:
	SwapChain(Device *pDevice, HWND hwnd, DXGI_FORMAT renderTargetFormat);
	void resize(uint32 width, uint32 height);
private:
	void updateBuffer();
private:
	Device  *_pDevice;
	CommandQueue  *_pCommandQueue;
	uint32	_width;
	uint32  _height;
	DXGI_FORMAT  _renderTargetFormat;
	std::size_t  _currentBackBufferIndex;
	WRL::ComPtr<ID3D12Resource>   _pSwapChainBuffer[kSwapChainBufferCount];
	WRL::ComPtr<ID3D12Resource>   _pDepthStencilBuffer;
	WRL::ComPtr< IDXGISwapChain>  _pSwapChain;
};

}