#pragma once
#include "dx12libCommon.h"
#include <memory>


namespace dx12lib {

class Adapter;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class SwapChain;
class CommandQueue;
class DescriptorAllocation;
class DescriptorAllocator;

class Device : public std::enable_shared_from_this<Device> {
public:
	Device(std::shared_ptr<Adapter> pAdapter);
	~Device() = default;

	std::shared_ptr<SwapChain> createSwapChain(
		HWND hwnd,
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM,
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT
	) const;

	std::shared_ptr<VertexBuffer> createVertexBuffer(
		const void *pData,
		uint32 sizeInByte,
		uint32 vertexStride
	) const;

	std::shared_ptr<IndexBuffer> createIndexBuffer(
		const void *pData,
		uint32 sizeInByte,
		DXGI_FORMAT indexFormat
	) const;

	std::shared_ptr<ConstantBuffer> createConstantBuffer(
		const void *pData,
		uint32 sizeInByte
	) const;

	DescriptorAllocation allocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors = 1);

	void releaseStaleDescriptor();

	UINT getSampleCount() const;
	UINT getSampleQuality() const;
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