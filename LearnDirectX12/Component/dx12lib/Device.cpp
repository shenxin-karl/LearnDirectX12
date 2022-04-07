#include "Device.h"
#include "Adapter.h"
#include "SwapChain.h"
#include "CommandQueue.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "MakeObejctTool.hpp"

namespace dx12lib {
	
Device::Device(std::shared_ptr<Adapter> pAdapter)
: _pAdapter(pAdapter) {
}

Device::~Device() {
}

void Device::initialize(const DeviceInitDesc &desc) {
#if defined(DEBUG) || defined(_DEBUG)
	{
		WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	_initDesc = desc;

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

	_pCommandQueue = std::make_shared<MakeCommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_DIRECT);

	for (std::size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		_pDescriptorAllocators[i] = std::make_unique<MakeDescriptorAllocator>(
			weak_from_this(),
			static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i),
			50
		);
	}
}

void Device::destory() {
	_pCommandQueue->flushCommandQueue();
}

std::shared_ptr<SwapChain> Device::createSwapChain(HWND hwnd) const {
	return std::make_shared<MakeSwapChain>(
		const_cast<Device *>(this)->weak_from_this(),
		hwnd,
		_initDesc.backBufferFormat,
		_initDesc.depthStencilFormat
	);
}

std::shared_ptr<RootSignature> Device::createRootSignature(const D3D12_ROOT_SIGNATURE_DESC &desc) {
	return std::make_shared<MakeRootSignature>(weak_from_this(), desc);
}

std::shared_ptr<RootSignature> Device::createRootSignature(const RootSignatureDescHelper &desc) {
	return std::make_shared<MakeRootSignature>(weak_from_this(), desc.getRootSignatureDesc());
}

std::shared_ptr<GraphicsPSO> Device::createGraphicsPSO(const std::string &name) {
	return std::make_shared<MakeGraphicsPSO>(weak_from_this(), name);
}

std::shared_ptr<ComputePSO> Device::createComputePSO(const std::string &name) {
	return std::make_shared<MakeComputePSO>(weak_from_this(), name);
}

DescriptorAllocation Device::allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors /*= 1*/) {
	auto index = static_cast<std::size_t>(type);
	return _pDescriptorAllocators[index]->allocate(numDescriptors);
}

void Device::releaseStaleDescriptor() {
	for (auto &pAllocator : _pDescriptorAllocators)
		pAllocator->releaseStateDescriptors();
}

const dx12lib::DeviceInitDesc &Device::getDesc() const {
	return _initDesc;
}

std::shared_ptr<Adapter> Device::getAdapter() const {
	return _pAdapter;
}

std::shared_ptr<CommandQueue> Device::getCommandQueue() const {
	return _pCommandQueue;
}

ID3D12Device *Device::getD3DDevice() const {
	return _pDevice.Get();
}

}