#include "Device.h"
#include "Adapter.h"

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

std::shared_ptr<SwapChain> Device::createSwapChain() const {

}

std::shared_ptr<dx12lib::Adapter> Device::getAdapter() const {
	return _pAdapter;
}

std::shared_ptr<dx12lib::CommandQueue> Device::getCommandQueue(CommandQueueType type) const {
	return _pCommandQueueList[static_cast<std::size_t>(type)];
}

}