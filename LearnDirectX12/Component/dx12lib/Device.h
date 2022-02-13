#pragma once
#include "dx12libStd.h"
#include <memory>


namespace dx12lib {

class Device : public std::enable_shared_from_this<Device> {
public:
	Device(std::shared_ptr<Adapter> pAdapter);
	Device(const Device &) = delete;
	Device &operator=(const Device &) = delete;
	~Device();
	void initialize();
	void destory();

	std::shared_ptr<SwapChain> createSwapChain(
		HWND hwnd,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
	) const;

	std::shared_ptr<RootSignature> createRootSignature(const D3D12_ROOT_SIGNATURE_DESC &desc);
	std::shared_ptr<RootSignature> createRootSignature(const RootSignatureDescHelper &desc);
	std::shared_ptr<GraphicsPSO> createGraphicsPSO(const std::string &name);

	DescriptorAllocation allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors = 1);

	void releaseStaleDescriptor();

	UINT getSampleCount() const;
	UINT getSampleQuality() const;
	DXGI_SAMPLE_DESC getSampleDesc() const;
public:
	std::shared_ptr<Adapter> getAdapter() const;
	std::shared_ptr<CommandQueue> getCommandQueue(CommandQueueType type) const;
	ID3D12Device *getD3DDevice() const;
private:
	WRL::ComPtr<ID3D12Device>            _pDevice;
	std::shared_ptr<Adapter>             _pAdapter;
	std::shared_ptr<CommandQueue>        _pCommandQueueList[kComandQueueTypeCount];
	std::unique_ptr<DescriptorAllocator> _pDescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	UINT _4xMsaaQuality = 0;
	UINT _4xMsaaState = false;
};

}