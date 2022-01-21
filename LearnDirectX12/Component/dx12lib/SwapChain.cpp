#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include <string>

namespace dx12lib {

SwapChain::SwapChain(std::weak_ptr<Device> pDevice,
		HWND hwnd,
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat)
: _pDevice(pDevice), _renderTargetFormat(backBufferFormat)
, _depthStendilFormat(depthStencilFormat), _hwnd(hwnd)
, _currentBackBufferIndex(0)
{
	RECT windowRect;
	::GetClientRect(hwnd, &windowRect);
	_width = windowRect.right - windowRect.left;
	_height = windowRect.bottom - windowRect.top;

	auto pSharedDevice = pDevice.lock();
	_pSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = _width;
	sd.BufferDesc.Height = _height;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.Format = backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = pSharedDevice->getSampleCount();
	sd.SampleDesc.Quality = pSharedDevice->getSampleQuality();
	sd.OutputWindow = hwnd;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = kSwapChainBufferCount;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	CommandListProxy pCmdList = pSharedDevice->getCommandQueue(CommandQueueType::Direct)->createCommandListProxy();
	auto *pDxgiFactory = pSharedDevice->getAdapter()->getDxgiFactory();
	ThrowIfFailed(pDxgiFactory->CreateSwapChain(
		pCmdList->getD3DCommandList(),
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

DXGI_FORMAT SwapChain::getRenderTargetFormat() const {
	return _renderTargetFormat;
}

DXGI_FORMAT SwapChain::getDepthStencilFormat() const {
	return _depthStendilFormat;
}


UINT SwapChain::present() {
	auto errorCode = _pSwapChain->Present(0, 0);
	_currentBackBufferIndex = (_currentBackBufferIndex + 1) % kSwapChainBufferCount;
	return errorCode;
}

void SwapChain::updateBuffer() {
	for (std::size_t i = 0; i < kSwapChainBufferCount; ++i) {
		WRL::ComPtr<ID3D12Resource> pBuffer;
		ThrowIfFailed(_pSwapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&pBuffer)));
		std::wstring name = L"BackBuffer[";
		name.append(L"i");
		name.append(L"]");
		pBuffer->SetName(name.c_str());
		// todo: initialize pSwapChainBuffer
	}
}

}