#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandQueue.h"
#include <string>

namespace dx12lib {

SwapChain::SwapChain(Device *pDevice,
		HWND hwnd,
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat)
: _pDevice(pDevice), _renderTargetFormat(backBufferFormat)
, _depthStendilFormat(depthStencilFormat), _hwnd(hwnd)
{
	RECT windowRect;
	::GetClientRect(hwnd, &windowRect);
	_width = windowRect.right - windowRect.left;
	_height = windowRect.bottom - windowRect.top;

	_pSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = _width;
	sd.BufferDesc.Height = _height;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.Format = backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = pDevice->getSampleCount();
	sd.SampleDesc.Quality = pDevice->getSampleQuality();
	sd.OutputWindow = hwnd;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = kSwapChainBufferCount;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	auto *pDxgiFactory = pDevice->getAdapter()->getDxgiFactory();
	ThrowIfFailed(pDxgiFactory->CreateSwapChain(
		_pCommandQueue->getD3D12CommandQueue(),
		&sd,
		&_pSwapChain
	));

}

void SwapChain::resize(uint32 width, uint32 height) {
	if (width == _width && height == _height)
		return;

	_width = std::max(width, uint32(1));
	_height = std::max(height, uint32(1));
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	ThrowIfFailed(_pSwapChain->GetDesc(&swapChainDesc));
	ThrowIfFailed(_pSwapChain->ResizeBuffers(
		kSwapChainBufferCount, 
		_width,
		_height, 
		swapChainDesc.BufferDesc.Format, 
		swapChainDesc.Flags
	));
	updateBuffer();
}

std::shared_ptr<Texture> SwapChain::getRenderTarget() const {
	return _pSwapChainBuffer[_currentBackBufferIndex];
}

std::shared_ptr<Texture> SwapChain::getDepthStencil() const {
	return _pDepthStencilBuffer;
}

DXGI_FORMAT SwapChain::getRenderTargetFormat() const {
	return _renderTargetFormat;
}

DXGI_FORMAT SwapChain::getDepthStencilFormat() const {
	return _depthStendilFormat;
}


UINT SwapChain::present() {

}

void SwapChain::updateBuffer() {
	for (std::size_t i = 0; i < kSwapChainBufferCount; ++i) {
		WRL::ComPtr<ID3D12Resource> pBuffer;
		ThrowIfFailed(_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBuffer)));
		std::wstring name = L"BackBuffer[";
		name.append(L"i");
		name.append(L"]");
		pBuffer->SetName(name.c_str());
		// todo: 初始化 _pSwapChainBuffer
	}

	// todo: 初始化: _pDepthStencilBuffer
}

}