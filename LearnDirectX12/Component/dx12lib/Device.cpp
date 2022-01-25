#include "Device.h"
#include "Adapter.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"

namespace dx12lib {
	
Device::Device(std::shared_ptr<Adapter> pAdapter)
: _pAdapter(pAdapter) {
}

void Device::initialize() {
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
		ThrowIfFailed(_pAdapter->getDxgiFactory()->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&_pDevice)
		));
	}

	auto pDirectQueue = std::make_shared<CommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	_pCommandQueueList[std::size_t(CommandQueueType::Direct)] = pDirectQueue;
	auto pComputeQueue = std::make_shared<CommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	_pCommandQueueList[std::size_t(CommandQueueType::Compute)] = pComputeQueue;
	auto pCopyQueue = std::make_shared<CommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	_pCommandQueueList[std::size_t(CommandQueueType::Copy)] = pCopyQueue;

	for (std::size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		_pDescriptorAllocators[i] = std::make_unique<DescriptorAllocator>(
			weak_from_this(),
			static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i),
			50
		);
	}
}

std::shared_ptr<SwapChain> Device::createSwapChain(
		HWND hwnd,
		DXGI_FORMAT backBufferFormat,
		DXGI_FORMAT depthStencilFormat) const 
{
	return std::make_shared<SwapChain>(

		const_cast<Device*>(this)->weak_from_this(),
		hwnd, 
		backBufferFormat, 
		depthStencilFormat
	);
}

std::shared_ptr<dx12lib::VertexBuffer> 
Device::createVertexBuffer(const void *pData, uint32 sizeInByte, uint32 vertexStride) const {
	// todo
	return nullptr;
}

DescriptorAllocation Device::allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors /*= 1*/) {
	auto index = static_cast<std::size_t>(type);
	return _pDescriptorAllocators[index]->allocate(numDescriptors);
}

void Device::releaseStaleDescriptor() {
	for (auto &pAllocator : _pDescriptorAllocators)
		pAllocator->releaseStateDescriptors();
}

UINT Device::getSampleCount() const {
	return _4xMsaaState ? 4 : 1;
}

UINT Device::getSampleQuality() const {
	return _4xMsaaState ? (_4xMsaaQuality-1) : 0;
}

DXGI_SAMPLE_DESC Device::getSampleDesc() const {
	return {
		getSampleCount(),
		getSampleQuality(),
	};
}

std::shared_ptr<Adapter> Device::getAdapter() const {
	return _pAdapter;
}

std::shared_ptr<CommandQueue> Device::getCommandQueue(CommandQueueType type) const {
	return _pCommandQueueList[static_cast<std::size_t>(type)];
}

ID3D12Device *Device::getD3DDevice() const {
	return _pDevice.Get();
}

}