#include "Device.h"
#include "Adapter.h"
#include "SwapChain.h"

namespace dx12lib {
	
Device::Device(std::shared_ptr<Adapter> pAdapter)
: _pAdapter(pAdapter) {
#if defined(DEBUG) || defined(_DEBUG)
	{
		WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_pDevice));
	if (FAILED(hr)) {
		WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(pAdapter->getDxgiFactory()->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&_pDevice)
		));
	}
}

std::shared_ptr<SwapChain> Device::createSwapChain(
		HWND hwnd,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT) const 
{
	return std::make_shared<SwapChain>(
		const_cast<Device *>(this),
		hwnd, 
		backBufferFormat, 
		depthStencilFormat
	);
}

UINT Device::getSampleCount() const {
	return _4xMsaaState ? 4 : 1;
}

UINT Device::getSampleQuality() const {
	return _4xMsaaState ? (_4xMsaaQuality-1) : 0;
}

std::shared_ptr<dx12lib::Adapter> Device::getAdapter() const {
	return _pAdapter;
}

std::shared_ptr<dx12lib::CommandQueue> Device::getCommandQueue(CommandQueueType type) const {
	return _pCommandQueueList[static_cast<std::size_t>(type)];
}

}