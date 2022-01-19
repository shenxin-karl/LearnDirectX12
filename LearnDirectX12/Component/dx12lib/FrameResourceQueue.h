#pragma once
#include "dx12libCommon.h"

namespace dx12lib {

class CommandList;
class FrameResourceItem {
public:
	FrameResourceItem(std::weak_ptr<Device> pDevice);
	FrameResourceItem(const FrameResourceItem &) = delete;
	FrameResourceItem &operator=(const FrameResourceItem &) = delete;
	uint64 getFence() const noexcept;
	void setFence(uint64 fence) noexcept;
	std::shared_ptr<CommandList> createCommandList();
private:
	uint64                                     _fence = 0;
	std::weak_ptr<Device>                      _pDevice;
	WRL::ComPtr<ID3D12CommandAllocator>        _pCmdListAlloc;
	std::vector<std::shared_ptr<CommandList>>  _cmdListPool;
	std::vector<std::shared_ptr<CommandList>>  _availableCmdList;
};

class FrameResourceQueue {
public:
	std::shared_ptr<CommandList> createCommandList();
private:
	uint32 _currentFrameResourceIndex;
	std::array<FrameResourceItem, kFrameResourceCount>  _frameResourceQueue;
};



}