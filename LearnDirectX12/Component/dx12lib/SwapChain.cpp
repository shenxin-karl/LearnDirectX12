#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandQueue.h"
#include <string>

namespace dx12lib {

SwapChain::SwapChain(Device *pDevice, HWND hwnd, DXGI_FORMAT renderTargetFormat)
: _pDevice(pDevice), _renderTargetFormat(renderTargetFormat)
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
	sd.BufferDesc.Format = renderTargetFormat;
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

void SwapChain::updateBuffer() {
	for (std::size_t i = 0; i < kSwapChainBufferCount; ++i) {
		ThrowIfFailed(_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&_pSwapChainBuffer[i])));
		std::wstring name = L"BackBuffer[";
		name.append(L"i");
		name.append(L"]");
		_pSwapChainBuffer[i]->SetName(name.c_str());
	}
}

}