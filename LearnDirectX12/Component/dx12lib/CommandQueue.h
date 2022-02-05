#pragma once
#include "dx12libStd.h"
#include "CommandListProxy.h"
#include "FrameResourceQueue.h"

namespace dx12lib {

class SwapChain;
class CommandList;
class FrameResourceQueue;
class CommandQueue {
public:
	CommandQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE queueType);
	CommandQueue(const CommandQueue &) = delete;
	ID3D12CommandQueue *getD3D12CommandQueue() const;
	uint64 signal(std::shared_ptr<SwapChain> pSwapChain);
	void executeCommandList(CommandListProxy pCommandList);
	void executeCommandList(const std::vector<CommandListProxy> &cmdLists);
	bool isFenceComplete(uint64 fenceValue) const noexcept;
	void waitForFenceValue(uint64 fenceValue);
	uint32 getFrameResourceCount() const;
	uint64 getFenceValue() const;
	uint64 getCompletedValue() const;
	CommandListProxy createCommandListProxy();
	void newFrame();
	void resize(uint32 width, uint32 height, std::shared_ptr<SwapChain> pSwapChain);
	void flushCommandQueue();
	~CommandQueue();
private:
	uint64                              _fenceValue;
	std::weak_ptr<Device>               _pDevice;
	D3D12_COMMAND_LIST_TYPE             _queueType;
	WRL::ComPtr<ID3D12Fence>            _pFence;
	WRL::ComPtr<ID3D12CommandQueue>     _pCommandQueue;
	std::unique_ptr<FrameResourceQueue> _pFrameResourceQueue;
};

}