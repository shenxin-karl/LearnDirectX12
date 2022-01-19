#pragma once
#include "dx12libCommon.h"
#include "ThreakSafeQueue.hpp"

namespace dx12lib {

class SwapChain;
class CommandQueue {
public:
	CommandQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE queueType);
	CommandQueue(const CommandQueue &) = delete;
	ID3D12CommandQueue *getD3D12CommandQueue() const;
	uint64 signal(std::shared_ptr<SwapChain> pSwapChain);
	void executeCommandList(std::shared_ptr<CommandList> pCommandList);
	void executeCommandList(const std::vector<std::shared_ptr<CommandList>> &cmdLists);
	bool isFenceComplete(uint64 fenceValue) const noexcept;
	void waitForFenceValue(uint64 fenceValue);
private:
	uint64                          _fenceValue;
	std::weak_ptr<Device>           _pDevice;
	D3D12_COMMAND_LIST_TYPE         _queueType;
	WRL::ComPtr< ID3D12Fence>       _pFence;
	WRL::ComPtr<ID3D12CommandQueue> _pCommandQueue;
};

}