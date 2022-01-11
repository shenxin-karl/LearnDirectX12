#pragma once
#include "dx12libCommon.h"
#include <memory>


namespace dx12lib {

class Adapter;
class VertexBuffer;
class IndexBuffer;
class SwapChain;
class CommandQueue;

class Device {
public:
	Device(std::shared_ptr<Adapter> pAdapter);
	~Device() = default;
	std::shared_ptr<VertexBuffer> createVertexBuffer() const;
	std::shared_ptr<IndexBuffer> createIndexBuffer() const;
	std::shared_ptr<SwapChain> createSwapChain() const;
	UINT getSampleCount() const;
	UINT getSampleQuality() const;
public:
	std::shared_ptr<Adapter> getAdapter() const;
	std::shared_ptr<CommandQueue> getCommandQueue(CommandQueueType type) const;
private:
	WRL::ComPtr<ID3D12Device>     _pDevice;
	std::shared_ptr<Adapter>      _pAdapter;
	std::shared_ptr<CommandQueue> _pCommandQueueList[kComandQueueTypeCount];
};

}