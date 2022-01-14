#pragma once
#include "dx12libCommon.h"
#include "ThreakSafeQueue.hpp"

namespace dx12lib {

class CommandList;
class CommandQueue {
public:
	CommandQueue(std::shared_ptr<Device> pDevice);
	ID3D12CommandQueue *getD3D12CommandQueue() const;
	void signal();
	uint64 executeCommandList(std::shared_ptr<CommandList> pCommandList);
	uint64 executeCommandList(const std::vector<std::shared_ptr<CommandList>> &cmdLists);
	bool isFenceComplete(uint64 fenceValue) const noexcept;
	void waitForFenceValue(uint64 fenceValue);
	void fluse();
	std::shared_ptr<CommandList> getCommandList() const;
private:
	std::weak_ptr<Device> _pDevice;
	std::atomic<uint64>	_fenceValue;
	WRL::ComPtr<ID3D12CommandQueue> _pCommandQueue;
	ThreadSafeQueue<std::shared_ptr<CommandList>>  _commandLists;
	ThreadSafeQueue<std::shared_ptr<CommandList>>  _availableCommandLists;
};

}