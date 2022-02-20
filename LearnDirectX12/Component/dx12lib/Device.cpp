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

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = desc.backBufferFormat;	
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(_pDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)
	));
	_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(_4xMsaaQuality > 0 && "Unexpected MSAA quality level");

	auto pDirectQueue = std::make_shared<MakeCommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	_pCommandQueueList[std::size_t(CommandQueueType::Direct)] = pDirectQueue;
	auto pComputeQueue = std::make_shared<MakeCommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	_pCommandQueueList[std::size_t(CommandQueueType::Compute)] = pComputeQueue;
	auto pCopyQueue = std::make_shared<MakeCommandQueue>(weak_from_this(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
	_pCommandQueueList[std::size_t(CommandQueueType::Copy)] = pCopyQueue;

	for (std::size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		_pDescriptorAllocators[i] = std::make_unique<MakeDescriptorAllocator>(
			weak_from_this(),
			static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i),
			50
		);
	}
}

void Device::destory() {
	for (auto &pCmdQueue : _pCommandQueueList)
		pCmdQueue->flushCommandQueue();
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
	return std::make_shared<MakeGraphicsPSO>(name);
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

void Device::set4xMsaaState(bool state) {
	_4xMsaaState = true;
}

bool Device::get4xMsaaState() const {
	return _4xMsaaState;
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