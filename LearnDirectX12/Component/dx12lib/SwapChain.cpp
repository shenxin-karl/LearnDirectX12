#include "SwapChain.h"
#include "Device.h"
#include "Adapter.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Texture.h"
#include "RenderTarget.h"
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
	auto width = windowRect.right - windowRect.left;
	auto height = windowRect.bottom - windowRect.top;

	auto pSharedDevice = pDevice.lock();
	_pSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
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

	auto pCmdQueue = pSharedDevice->getCommandQueue(CommandQueueType::Direct);
	auto *pDxgiFactory = pSharedDevice->getAdapter()->getDxgiFactory();
	ThrowIfFailed(pDxgiFactory->CreateSwapChain(
		pCmdQueue->getD3D12CommandQueue(),
		&sd,
		&_pSwapChain
	));
}

void SwapChain::resize(CommandListProxy pCmdList, uint32 width, uint32 height) {
	if (width == _width && height == _height)
		return;

	_pDepthStencilBuffer = nullptr;
	for (auto &pTexture : _pSwapChainBuffer)
		pTexture = nullptr;

	_pRenderTarget = std::make_shared<RenderTarget>(width, height);
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
	updateBuffer(pCmdList);
}

DXGI_FORMAT SwapChain::getRenderTargetFormat() const {
	return _renderTargetFormat;
}

DXGI_FORMAT SwapChain::getDepthStencilFormat() const {
	return _depthStendilFormat;
}

void SwapChain::present() {
	ThrowIfFailed(_pSwapChain->Present(0, 0));
	_currentBackBufferIndex = (_currentBackBufferIndex + 1) % kSwapChainBufferCount;
	_pRenderTarget->attachTexture(AttachmentPoint::Color0, getCurrentBackBuffer());
}

std::shared_ptr<RenderTarget> SwapChain::getRenderTarget() const {
	return _pRenderTarget;
}

std::shared_ptr<Texture> SwapChain::getCurrentBackBuffer() const {
	return _pSwapChainBuffer[_currentBackBufferIndex];
}

void SwapChain::updateBuffer(CommandListProxy pCmdList) {
	for (std::size_t i = 0; i < kSwapChainBufferCount; ++i) {
		WRL::ComPtr<ID3D12Resource> pBuffer;
		ThrowIfFailed(_pSwapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&pBuffer)));
		std::wstring name = L"BackBuffer[";
		name.append(std::to_wstring(i));
		name.append(L"]");
		pBuffer->SetName(name.c_str());
		_pSwapChainBuffer[i] = std::make_shared<Texture>(_pDevice, pBuffer, nullptr);
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = _width;
	depthStencilDesc.Height = _height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = _depthStendilFormat;
	depthStencilDesc.SampleDesc = _pDevice.lock()->getSampleDesc();
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = _depthStendilFormat;
	optClear.DepthStencil.Depth = 1.f;
	optClear.DepthStencil.Stencil = 0;
	_pDepthStencilBuffer = std::make_shared<Texture>(_pDevice, depthStencilDesc, &optClear);
	_pDepthStencilBuffer->getD3DResource()->SetName(L"DepthStencilBuffer");
	pCmdList->transitionBarrier(_pDepthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	_pRenderTarget->attachTexture(AttachmentPoint::Color0, getCurrentBackBuffer());
	_pRenderTarget->attachTexture(AttachmentPoint::DepthStencil, _pDepthStencilBuffer);
}

}