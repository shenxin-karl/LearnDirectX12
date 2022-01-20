#pragma once
#include "dx12libCommon.h"
#include <array>

namespace dx12lib {

class CommandList;
class FrameResourceItem {
public:
	FrameResourceItem(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
	FrameResourceItem(const FrameResourceItem &) = delete;
	FrameResourceItem &operator=(const FrameResourceItem &) = delete;
	uint64 getFence() const noexcept;
	void setFence(uint64 fence) noexcept;
	std::shared_ptr<CommandList> createCommandList();
	void reset();
private:
	uint64                                     _fence = 0;
	D3D12_COMMAND_LIST_TYPE                    _cmdListType;
	std::weak_ptr<Device>                      _pDevice;
	WRL::ComPtr<ID3D12CommandAllocator>        _pCmdListAlloc;
	std::vector<std::shared_ptr<CommandList>>  _cmdListPool;
	std::vector<std::shared_ptr<CommandList>>  _availableCmdList;
};

class FrameResourceQueue {
public:
	FrameResourceQueue(std::weak_ptr<Device> pDevice, D3D12_COMMAND_LIST_TYPE cmdListType);
	std::shared_ptr<CommandList> createCommandList();
	uint32 getMaxFrameResourceCount() const noexcept;
	void newFrame(uint64 fence);
private:
	uint32 _currentFrameResourceIndex;
	uint32 _frameResourceItemCount;
	std::array<std::unique_ptr<FrameResourceItem>, kFrameResourceCount>  _frameResourceQueue;
};



}