#pragma once
#include "dx12libCommon.h"
#include "ThreakSafeQueue.hpp"

namespace dx12lib {

class CommandList;
class FrameResource;
class CommandQueue {
public:
	CommandQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE queueType);
	CommandQueue(const CommandQueue &) = delete;
	ID3D12CommandQueue *getD3D12CommandQueue() const;
	void signal();
	uint64 executeCommandList(std::shared_ptr<CommandList> pCommandList);
	uint64 executeCommandList(const std::vector<std::shared_ptr<CommandList>> &cmdLists);
	bool isFenceComplete(uint64 fenceValue) const noexcept;
	void waitForFenceValue(uint64 fenceValue);
	std::shared_ptr<CommandList> getCommandList() const;
	std::shared_ptr<FrameResource> getCurrentFrameResource() const;
	uint32 getCurrentFrameResourceIndex() const;
private:
	D3D12_COMMAND_LIST_TYPE _queueType;
	std::weak_ptr<Device> _pDevice;
	uint64	_fenceValue;
	WRL::ComPtr<ID3D12CommandQueue> _pCommandQueue;
	mutable ThreadSafeQueue<std::shared_ptr<CommandList>>  _commandLists;
	mutable	ThreadSafeQueue<std::shared_ptr<CommandList>>  _availableCommandLists;
	std::shared_ptr<FrameResource> _pFrameResource[kFrameResourceCount];
	uint32 _currentFrameResourceIndex;
};

}