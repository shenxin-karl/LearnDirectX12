#pragma once
#include "dx12libCommon.h"
#include "CommandListProxy.h"
#include "ThreakSafeQueue.hpp"
#include <array>

namespace dx12lib {

class CommandList;
class FrameResourceItem;
class CommandListProxy;
class FrameResourceItem : public std::enable_shared_from_this<FrameResourceItem> {
public:
	FrameResourceItem(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
	FrameResourceItem(const FrameResourceItem &) = delete;
	FrameResourceItem &operator=(const FrameResourceItem &) = delete;
	uint64 getFence() const noexcept;
	void setFence(uint64 fence) noexcept;
	CommandListProxy createCommandListProxy();
	void releaseCommandList(std::shared_ptr<CommandList> pCommandList);
private:
	uint64                                     _fence = 0;
	D3D12_COMMAND_LIST_TYPE                    _cmdListType;
	std::weak_ptr<Device>                      _pDevice;
	WRL::ComPtr<ID3D12CommandAllocator>        _pCmdListAlloc;
	mutable ThreadSafeQueue<std::shared_ptr<CommandList>>  _cmdListPool;
	mutable ThreadSafeQueue<std::shared_ptr<CommandList>>  _availableCmdList;
};


class FrameResourceQueue {
public:
	FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
	CommandListProxy createCommandListProxy();
	uint32 getMaxFrameResourceCount() const noexcept;
	void newFrame(uint64 fence);
private:
	uint32 _currentFrameResourceIndex;
	uint32 _frameResourceItemCount;
	std::array<std::unique_ptr<FrameResourceItem>, kFrameResourceCount>  _frameResourceQueue;
};



}